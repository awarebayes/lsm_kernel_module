"""
Тест запрещает процессу доступ ко всем файлам.
"""

import subprocess
import time
import os


def cmd(s):
    print(f"$ {s}")
    os.system(s)


print("Running read_file.py")
proc = subprocess.Popen(
    ["python ./read_file.py"],
    shell=True,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    stdin=subprocess.PIPE,
)
pid = proc.pid

time.sleep(0.1)
cmd(f'echo "restrict {pid}" > /proc/mylsm/mylsm')

output, err = proc.communicate(input=b"run")
print("Output:")
print(output.decode())
print("Error:")
print(err.decode())
print("Return code")
print(proc.returncode)

os.system(f'echo "unrestrict {pid}" > /proc/mylsm/mylsm')
