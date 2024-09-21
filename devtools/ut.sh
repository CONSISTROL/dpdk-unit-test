#!/bin/bash

set -e

SCRIPT_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# 定义命令及其描述
declare -A commands=(
    [build]="开始构建项目"
    [run]="运行项目"
    [help]="显示此帮助信息"
)

# 定义简写命令
declare -A short_commands=(
    [b]="build"
    [r]="run"
    [h]="help"
)

# 输出可用选项和对应命令的帮助信息
help() {
    echo "可用选项:"
    for short in "${!short_commands[@]}"; do
        cmd="${short_commands[$short]}"
        echo "  $short | $cmd  ${commands[$cmd]}"
    done
}

build() {
    echo "开始构建..."
}

run() {
    echo "开始运行..."
}

cmd="$1"
# 如果是简写形式，则转换
if [[ -v short_commands[$cmd] ]]; then
    cmd="${short_commands[$cmd]}"
fi

if [[ -v commands[$cmd] ]]; then
    "$cmd"
else
    echo "无效参数: '$cmd'"
    help
    exit 1
fi
