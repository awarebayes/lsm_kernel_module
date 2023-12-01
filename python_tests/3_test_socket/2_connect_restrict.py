"""
Тест запрещает процессу доступ ко всем ip.
Процесс обращается к  публичному  ip google.
Получает  operation not permitted.
"""


import subprocess
import time
import os


def cmd(s):
    print(f"$ {s}")
    os.system(s)


print("Running connect_to_port.py")
proc = subprocess.Popen(
    ["python connect_to_port.py"],
    shell=True,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    stdin=subprocess.PIPE,
)
time.sleep(0.1)

GOOGLE_IP = "216.58.200.227:80"

pid = proc.pid
cmd(f'echo "restrict {pid}" > /proc/mylsm/mylsm')

output, err = proc.communicate(input=GOOGLE_IP.encode())
print("Output:")
print(output.decode())
print("Error:")
print(err.decode())
print("Return code")
print(proc.returncode)

cmd(f'echo "debug 0" > /proc/mylsm/mylsm')
cmd(f'echo "unrestrict {pid}" > /proc/mylsm/mylsm')
