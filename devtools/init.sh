#!/bin/bash

SCRIPT_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
PROJECT_PATH="$(dirname "$SCRIPT_PATH")"

# 定义输出分割线的函数
print_separator() {
    local char=${1:-'-'}  # 默认字符为短横线
    local length=${2:-70}  # 默认长度为 70

    printf '%*s\n' "$length" '' | tr ' ' "$char"
}

print_separator '='
alias build="python3 ${PROJECT_PATH}/build.py"
echo "use 'build' to run ${PROJECT_PATH}/build.py"

print_separator
alias run="${PROJECT_PATH}/a.out"
echo "use 'run' to run ${PROJECT_PATH}/a.out"

print_separator
alias sdv="sh ${SCRIPT_PATH}/sdv.sh"
echo "use 'sdv' to run ${SCRIPT_PATH}/sdv.sh"
sdv help

print_separator
alias ut="sh ${SCRIPT_PATH}/ut.sh"
echo "use 'ut' to run ${SCRIPT_PATH}/ut.sh"
ut help
