/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bindSocket.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 19:43:25 by cmakario          #+#    #+#             */
/*   Updated: 2025/08/05 20:22:34 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "bindSocket.hpp"

// The constructor of basicSocket already handles the socket creation and binding.
bindSocket::bindSocket(int domain, int service, int protocol, int port, u_long interface)
	: basicSocket(domain, service, protocol, port, interface)
	{
		setBindResult(connectTheSocket(getSocketFd(), getAddress()));
		testConnection(getBindResult());
	}

int bindSocket::connectTheSocket(int sock, struct sockaddr_in address) const
{
	// Connect the socket to the address
	return bind(socket_fd, (struct sockaddr *)&address, sizeof(address));
}