#ifndef BASIC_SERVER_HPP
#define BASIC_SERVER_HPP

#include "../Sockets/lib-sockets.hpp"


class basicServer {
private:
	listenSocket* socket;
	virtual void accepter() = 0; // Accept connections
	virtual void handler() = 0; // Handle requests
	virtual void responder() = 0; // Send responses
public:
	// Constructor
	basicServer(int domain, int service, int protocol, int port, u_long interface, int backlog);
	virtual void launch() = 0; // Launch the server
	// Getters
	listenSocket* getSocket() const;
};

#endif // !BASIC_SERVER_HPP
