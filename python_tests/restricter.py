import os
from dataclasses import dataclass
from typing import List, Tuple, Optional


@dataclass
class RestrictionInfo:
    pid: int
    allowed_files: List[Tuple[str, str]]
    allowed_ips: List[str]
    working_dir: Optional[str]


def read_seq_proc():
    os.system(f"cat /proc/mylsm/mylsm > /tmp/mybuffer")
    with open("/tmp/mybuffer", "r") as f:
        return f.read()


def write_seq_proc(command):
    os.system(f'echo "{command}" > /proc/mylsm/mylsm')


def get_restricted_pids():
    pids = []
    write_seq_proc("info_pids 0")
    lines = read_seq_proc().split("\n")
    for line in lines:
        line = line.strip()
        if line == "pids start":
            continue
        if line == "pids end":
            break
        pid = int(line)
        pids.append(pid)
    return pids


def get_restriction_info(pid):
    result = RestrictionInfo(0, [], [], None)
    write_seq_proc(f"info_restriction {pid}")
    lines = read_seq_proc().split("\n")
    for line in lines:
        line = line.strip()
        if line == "restriction start":
            continue
        if line == "restriction end":
            break

        words = line.split()
        if words[0] == "pid":
            result.pid = int(words[1])

        if words[0] == "file":
            result.allowed_files.append((words[1], words[2]))

        if words[0] == "ipv4":
            result.allowed_ips.append(words[1])

        if words[0] == "dir":
            result.working_dir = words[1]
    return result


def print_processes():
    print("Restricted pids")
    for p in get_restricted_pids():
        print("p", p)
    print("Restricted pids ended")


def print_detailed():
    pids = get_restricted_pids()
    print("Restricted pids:", len(pids))
    for pid in pids:
        result = get_restriction_info(pid)
        print("---")
        print("pid", result.pid)
        print("Allowed files:", len(result.allowed_files))
        print("\n".join(map(str, result.allowed_files)))
        print("Allowed ips:", len(result.allowed_ips))
        print("\n".join(result.allowed_ips))
        print("Working dir:", result.working_dir)
        print("---")


def restrict_process():
    pid = int(input("Process id: "))
    write_seq_proc(f"restrict {pid}")


def modify_restricted_process():
    pid = int(input("Process id: "))
    option = 0
    while True:
        print("Options:")
        print("1 - allow file")
        print("2 - allow ipv4")
        print("3 - allow directory")
        option = int(input("Your choice: "))
        if option in [1, 2, 3]:
            break
    if option == 1:
        print("Input file path")
        file_path = input("File path: ")
        file_type = 0

        while True:
            print("Options:")
            print("1 - regular file")
            print("2 - file socket")
            print("3 - fifo pipe")
            file_type = int(input("Your choice: "))
            if file_type in [1, 2, 3]:
                break

        file_type_str = {1: "regular", 2: "unix_socket", 3: "pipe"}[file_type]
        write_seq_proc(f"allow_file {pid} {file_path} {file_type_str}")
    elif option == 2:
        ip = input("Ip V4 with port (127.0.0.1:80): ")
        write_seq_proc(f"allow_ip {pid} {ip}")
    elif option == 3:
        dir_input = input("Directory ending with /: ")
        write_seq_proc(f"allow_directory {pid} {dir_input}")


def unrestrict_process():
    pid = int(input("Process id: "))
    write_seq_proc(f"unrestrict {pid}")


while True:
    print("Menu:")
    print("1 - Print restricted processes (short)")
    print("2 - Print restricted processes (detailed)")
    print("3 - Restrict process")
    print("4 - Modify restricted process")
    print("5 - Unrestrict process")

    result = input("Your choice: ")
    try:
        result = int(result)
    except ValueError:
        print("Invalid option!")
        continue
    if result > 5 or result < 0:
        print("Invalid option!")
        continue

    if result == 1:
        print_processes()
    if result == 2:
        print_detailed()
    if result == 3:
        restrict_process()
    if result == 4:
        modify_restricted_process()
    if result == 5:
        unrestrict_process()
