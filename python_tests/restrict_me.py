import os
import socket


def connect_to_server(ip, port):
    client_socket = None
    try:
        port = int(port)
        # Create a socket object
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # Connect to the server
        server_address = (ip, port)
        client_socket.connect(server_address)

        print(f"Connected to {ip}:{port}")

        # You can send/receive data here

    except Exception as e:
        print(f"Error: {e}")

    finally:
        if socket is not None:
            client_socket.close()


def open_file(path):
    try:
        print("Accessing file", path)
        with open(path, "r") as f:
            print("Test file reads:", f.read())
    except Exception as e:
        print(f"Error: {e}")


while True:
    print("My pid:", os.getpid())
    action = input("Input action: [open, connect, exit]: ")
    if action == "open":
        file_to_open = input("Input file to open: ")
        open_file(file_to_open)
    elif action == "connect":
        server_ip, port = input("ip:port like 128.0.0.1:80 : ").split(":")
        print("Connecting to", server_ip, "on port", port)
        connect_to_server(server_ip, port)
    elif action == "exit":
        exit()
