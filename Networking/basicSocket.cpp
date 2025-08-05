/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   basicSocket.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 16:01:49 by cmakario          #+#    #+#             */
/*   Updated: 2025/08/05 16:53:03 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "basicSocket.hpp"
#include <sys/_endian.h>


// #include <unistd.h>

// Default Constructor
basicSocket::basicSocket(int domain, int service, int protocol, int port, u_long interface)
{
	// Initialize the address structure
	address.sin_family = domain;
	address.sin_port = htons(port); // Convert port to network byte order
	address.sin_addr.s_addr = htonl(interface); // Convert interface to network byte order
	// Socket creation
	socket_fd = socket(domain, service, protocol);
	testConnection(socket_fd);
	// if (socket_fd < 0)
	// {
	// 	perror("Socket creation failed");
	// 	exit(EXIT_FAILURE);
	// 	// throw std::runtime_error("Socket creation failed");
	// }
	// Bind the socket to the address
	// if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	// {
	// 	perror("Socket bind failed");
	// 	exit(EXIT_FAILURE);
	// 	// throw std::runtime_error("Socket bind failed");
	// }
	// Network connection creation
	bindResult = bind(socket_fd, (struct sockaddr *)&address, sizeof(address));
	testConnection(bindResult);
}
// TODO : better implementation for errors.
// Test Connection
void basicSocket::testConnection(int testConnectedObject) const
{
	if (testConnectedObject < 0)
	{
		perror("Connection failed...");
		exit(EXIT_FAILURE);
		// throw std::runtime_error("Invalid test object");
	}
}

// Getters
struct sockaddr_in basicSocket::getAddress() const
{
	return address;
}
int basicSocket::getSocketFd() const
{
	return socket_fd;
}
int basicSocket::getBindResult() const
{
	return bindResult;
}