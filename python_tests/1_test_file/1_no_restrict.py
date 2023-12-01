import subprocess
import os

print("Running read_file.py")
proc = subprocess.Popen(
    ["python ./read_file.py"],
    shell=True,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    stdin=subprocess.PIPE,
)

output, err = proc.communicate(input=b"run")
print("Output:")
print(output.decode())
print("Error:")
print(err.decode())
print("Return code")
print(proc.returncode)
