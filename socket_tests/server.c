
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>

int main() {
	// Create socket
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		perror("Socket creation failed");
		return 1;
	}

	// Set up address structure
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;	   // Set address family to AF_INET (IPv4)
	addr.sin_port = htons(8080);	 // Port in network byte order (8080)
	addr.sin_addr.s_addr = INADDR_ANY; // Listen on any available network interface

	// Bind to the specified address and port
	if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("Bind failed");
		return 1;
	}

	// Start listening for incoming connections
	if (listen(s, 10) < 0) {
		perror("Listen failed");
		return 1;
	}

	printf("Listening on port 8080...\n");

	// Accept a connection from a client
	int client_fd = accept(s, NULL, NULL);
	if (client_fd < 0) {
		perror("Accept failed"); 
		return 1;
	}

	// Receive request from client
	char buffer[256] = {0};
	recv(client_fd, buffer, 256, 0);

	// Extract requested file path (GET /file.html ...)
	char* f = buffer + 5;
	*strchr(f, ' ') = 0;

	// Open requested file
	int opened_fd = open(f, O_RDONLY);
	if (opened_fd < 0) {
		perror("File not found");
		return 1;
	}

	char file_buffer[256];
	int bytes_read = read(opened_fd, file_buffer, 256);
	if (bytes_read > 0) {
		// Send file contents to the client
		write(client_fd, file_buffer, bytes_read);
	}

	close(opened_fd);
	close(client_fd);
	close(s);

	return 0;
}


// Download options
//  curl -o index.html http://localhost:8080/index.html

// The command `curl -o index.html http://localhost:8080/index.html` does the following:

// ## What it does:
// 1. **Makes an HTTP GET request** to `http://localhost:8080/index.html`
// 2. **Downloads the response** from your server
// 3. **Saves the content** to a local file named index.html

// ## Breaking down the command:
// - `curl` - HTTP client tool
// - `-o index.html` - Output flag: saves response to file "index.html"
// - `http://localhost:8080/index.html` - URL to request

// ## What happens with your server:
// 1. Your server receives: `GET index.html HTTP/1.1`
// 2. Extracts index.html from the request
// 3. Opens the file index.html from the current directory
// 4. Reads its contents and sends them back
// 5. curl saves this response to a new index.html file

// ## Requirements:
// You need an index.html file in the same directory as your server executable for this to work. Without it, your server will print "File not found" and exit.

// To test, create a simple HTML file first:
// ```bash
// echo "<h1>Hello World</h1>" > index.html
// ```

// Then run your server and use the curl command.