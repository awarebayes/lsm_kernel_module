import socket


def connect_to_server(ip, port):
    client_socket = None
    try:
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


server_ip, port = input().split(":")
port = int(port)
print("Connecting to", server_ip, "on port", port)
connect_to_server(server_ip, port)
