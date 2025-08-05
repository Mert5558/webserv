/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   basicSocket.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 15:53:55 by cmakario          #+#    #+#             */
/*   Updated: 2025/08/06 00:16:08 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BASIC_SOCKET_HPP
#define BASIC_SOCKET_HPP

#include <stdio.h>
#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in structure
#include <iostream>

// Inside this class we will create a basic socket that can be used for networking.
// Specifically, it will handle the creation of a socket, binding it to an address, and testing the connection.
class basicSocket {
private:
		struct sockaddr_in	address;
		int					socket_fd;
		int					bindResult;
public:
		// Constructor
		basicSocket(int domain, int service, int protocol, int port, u_long interface);
		// Connect through bind
		virtual int bindTheSocket(int sock, struct sockaddr_in address) const = 0;
		// Test socket creation and bind
		void testConnection(int testConnectedObject) const;
		// Getters
		struct sockaddr_in getAddress() const;
		int getSocketFd() const;
		int getBindResult() const;
		// Setters
		void setBindResult(int bindRslt);
};

#endif // BASIC_SOCKET_HPP