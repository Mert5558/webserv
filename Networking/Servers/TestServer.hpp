#ifndef TEST_SERVER_HPP
#define TEST_SERVER_HPP

#include "BasicServer.hpp"
#include <unistd.h>

class testServer : public basicServer {
private:
	char buffer[4096] = {0}; // Buffer for incoming data
	int clientSocket; // Client socket for communication
	// Private methods for handling server operations
	void accepter() override;
	void handler() override ;
	void responder() override;
public:
	// Constructor
	testServer();
	void launch() override; // Launch the server


};
#endif // TEST_SERVER_HPP