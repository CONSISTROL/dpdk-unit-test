import subprocess

# 编译 C 程序
compile_process = subprocess.Popen(['gcc', 'main.c'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
stdout, stderr = compile_process.communicate()

# 检查编译是否成功
if compile_process.returncode != 0:
    print("编译错误：")
    print(stderr.decode())
