/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   listenSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 21:53:51 by cmakario          #+#    #+#             */
/*   Updated: 2025/08/05 22:21:59 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ListenSocket.hpp"

// Constructor
listenSocket::listenSocket(int domain, int service, int protocol, int port, u_long interface, int bcklg)
	: bindSocket(domain, service, protocol, port, interface)
{
	backlog = bcklg;
	startlinstening();
	testConnection(listening);
}

void listenSocket::startlinstening()
{
	// Start listening on the socket
	listening = listen(getSocketFd(), backlog);
}