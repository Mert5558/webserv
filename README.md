# 🌐 Webserv

A lightweight HTTP server written in **C++**, developed as part of the **Webserv project at 42 School**.  
The goal of this project is to understand how web servers work internally by implementing an **HTTP server from scratch**.

---

## 📚 About the Project

Webserv is a custom HTTP server capable of handling multiple clients, parsing HTTP requests, and serving static content.

It mimics the basic behavior of servers like **Nginx**, while following the constraints of **C++17** and the requirements defined by the **42 subject**.

The project focuses on:

- Network programming
- HTTP protocol implementation
- Event-driven architecture
- Process and file management
- Configuration file parsing

---

## ⚙️ Features

- HTTP/1.1 request handling
- Multiple client connections
- Non-blocking I/O
- Static file serving
- Custom configuration file
- Multiple server blocks
- Virtual hosts
- Error pages
- Directory listing
- File uploads
- CGI support

---

## 🏗 Architecture Overview

The server follows an **event-driven architecture**:

1. The server listens for incoming connections.
2. Clients send HTTP requests.
3. Requests are parsed and routed to the correct server block.
4. The server generates an HTTP response.
5. The response is sent back to the client.

Core components typically include:

- **Socket Manager** – Handles server sockets and client connections  
- **Request Parser** – Parses incoming HTTP requests  
- **Response Builder** – Generates valid HTTP responses  
- **Configuration Parser** – Reads server configuration files  
- **CGI Handler** – Executes external scripts  

---

## 📦 Installation

Clone the repository:

```bash
git clone https://github.com/Mert5558/webserv.git
cd webserv
```

Compile the project:

```bash
make
```

---

## 🚀 Usage

Run the server with a configuration file:

```bash
./webserv conf/default.conf
```

If no configuration file is provided, the server will use a default configuration.

Then open your browser and go to:

```
http://localhost:8080
```

---

## 🧪 Testing

You can test the server using:

- Web browsers
- `curl`
- `telnet`
- `netcat`

Example:

```bash
curl http://localhost:8080
```
