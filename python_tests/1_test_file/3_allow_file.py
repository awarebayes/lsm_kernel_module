"""
Тест разрешает процессу доступ к конкретному файлу.
"""


import subprocess
import time
import os


def cmd(s):
    print(f"$ {s}")
    os.system(s)


print("Running read_file.py")
proc = subprocess.Popen(
    ["python read_file.py"],
    shell=True,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    stdin=subprocess.PIPE,
)
time.sleep(0.1)

test_file_txt_path = "/tests/1_test_file/test.txt"
pid = proc.pid
cmd(f'echo "restrict {pid}" > /proc/mylsm/mylsm')
cmd(f'echo "allow_file {pid} {test_file_txt_path} regular" > /proc/mylsm/mylsm')

output, err = proc.communicate(input=b"run")
print("Output:")
print(output.decode())
print("Error:")
print(err.decode())
print("Return code")
print(proc.returncode)

cmd(f'echo "debug 0" > /proc/mylsm/mylsm')
cmd(f'echo "unrestrict {pid}" > /proc/mylsm/mylsm')
