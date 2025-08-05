import socket

host = "localhost"
port = 8081

# Create a socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((host, port))

# Send an HTTP request with an invalid version
request = "GET / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: close\r\n\r\n"
client_socket.sendall(request.encode())

# Receive the response
response = client_socket.recv(4096)
print(response.decode())

client_socket.close()