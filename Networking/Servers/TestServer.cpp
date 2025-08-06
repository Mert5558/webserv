#include "TestServer.hpp"
#include <sys/socket.h>

testServer::testServer() : basicServer(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY, 10)
{
	// Constructor initializes the server socket
	std::cout << "Server initialized on port 8080." << std::endl;
	launch(); // Start the server
}

void testServer::accepter()
{
	struct sockaddr_in	address = getSocket()->getAddress();
	int addrlen = sizeof(address);
	clientSocket = accept(getSocket()->getSocketFd(), (struct sockaddr *)&address ,(socklen_t *)&addrlen); 
	read(clientSocket, buffer, sizeof(buffer));
}

void testServer::handler()
{
	// Handle the request (for now, just print the received data)
	std::cout << "Received: \n" << buffer << std::endl;
}

void testServer::responder()
{
	// Send a response back to the client
	const char *response = "Hello from the server!----------------------------------";
	write(clientSocket, response, strlen(response));
	close(clientSocket); // Close the client socket after responding
}

void testServer::launch()
{
	while (true)
	{
		std::cout << "Waiting for a connection..." << std::endl;
		accepter(); // Accept a new connection
		handler();  // Handle the request
		responder(); // Send a response
		std::cout << "Response sent, waiting for the next connection..." << std::endl;
	}
}
