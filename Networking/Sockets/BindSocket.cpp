/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bindSocket.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 19:43:25 by cmakario          #+#    #+#             */
/*   Updated: 2025/08/05 22:22:14 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BindSocket.hpp"

// Constructor
bindSocket::bindSocket(int domain, int service, int protocol, int port, u_long interface) : basicSocket(domain, service, protocol, port, interface)
	{
		setBindResult(connectTheSocket(getSocketFd(), getAddress()));
		testConnection(getBindResult());
	}

// Defining connectTheSocket -> bind
int bindSocket::connectTheSocket(int sock, struct sockaddr_in address) const
{
	// Connect the socket to the address
	return bind(sock, (struct sockaddr *)&address, sizeof(address));
}