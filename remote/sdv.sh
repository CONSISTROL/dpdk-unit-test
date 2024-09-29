#!/bin/bash

param=$1
LOCKFILE="lockfile"

# 捕获脚本中断信号，确保锁定被释放
cleanup() {
    echo "Releasing lock and exiting..."
    flock -u 200
    exit
}

trap cleanup EXIT INT TERM

# 尝试创建锁文件，如果锁文件已存在，等待直到可以获取锁
exec 200>$LOCKFILE
flock 200 || { echo "Another script is using /sdv. Exiting."; exit 1; }

main() {
    echo ${param} start!
    rm -rf sdv
    sleep 1

    mkdir -p sdv
    touch "sdv/${param}"
    echo "${param}"
    sleep 1

    touch "sdv/${param}${param}"
    echo "${param}${param}"
    sleep 1

    echo ${param} finished!
}

main
