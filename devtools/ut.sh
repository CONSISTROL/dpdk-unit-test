#!/bin/bash

set -e

SCRIPT_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

help() {
    echo "可用选项:"
    echo "  build  开始构建项目"
    echo "  test   开始测试项目"
    echo "  run    运行项目"
    echo "  help   显示此帮助信息"
}

# 定义 build 函数
build() {
    echo "开始构建..."
    # 在这里添加构建命令
    # 示例：
    # make
}

# 定义 test 函数
test() {
    echo "开始测试..."
    # 在这里添加测试命令
    # 示例：
    # ./run_tests.sh
}

# 定义 run 函数
run() {
    echo "开始运行..."
    # 在这里添加运行命令
    # 示例：
    # ./my_program
}

# 检查输入参数并调用相应的函数
case "$1" in
    h | help)
        help
        ;;
    bd | build)
        build
        ;;
    t | test)
        test
        ;;
    r | run)
        run
        ;;
    *)
        echo "无效参数，请使用: build, test, 或 run"
        exit 1
        ;;
esac
