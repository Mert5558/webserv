#include "TestServer.hpp"

int main()
{
	// Create an instance of the testServer
	testServer server;

	// The server will run indefinitely, handling connections
	// and responding to clients.
	// To stop the server, you can use Ctrl+C in the terminal.
	server.launch();
	// Note: The server will run indefinitely until manually stopped.
	std::cout << "Server is running..." << std::endl;
	return 0; // Exit the program
}