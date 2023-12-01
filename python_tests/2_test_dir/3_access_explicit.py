"""
Тест разрешает процессу доступ к конкретной директории.
Процесс обращается к файлу в другой директории и получает  Permission Denied.
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


allowed_dir_path = "/tests/2_test_dir/allowed_dir/"
pid = proc.pid
cmd(f'echo "restrict {pid}" > /proc/mylsm/mylsm')
cmd(f'echo "allow_directory {pid} {allowed_dir_path}" > /proc/mylsm/mylsm')

explicit_file_txt_path = "/tests/2_test_dir/explicit_allow.txt"
cmd(f'echo "allow_file {pid} {explicit_file_txt_path} regular" > /proc/mylsm/mylsm')

testfile_txt_path = os.path.join(os.getcwd(), "./explicit_allow.txt")
output, err = proc.communicate(input=testfile_txt_path.encode())
print("Output:")
print(output.decode())
print("Error:")
print(err.decode())
print("Return code")
print(proc.returncode)

cmd(f'echo "debug 0" > /proc/mylsm/mylsm')
cmd(f'echo "unrestrict {pid}" > /proc/mylsm/mylsm')
