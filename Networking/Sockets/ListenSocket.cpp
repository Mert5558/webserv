/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   listenSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 21:53:51 by cmakario          #+#    #+#             */
/*   Updated: 2025/08/06 00:29:57 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ListenSocket.hpp"

// Constructor
listenSocket::listenSocket(int domain, int service, int protocol, int port, u_long interface, int backlog) : bindSocket(domain, service, protocol, port, interface)
{
	_backlog = backlog;
	startlinstening();
	testConnection(listening);
}

void listenSocket::startlinstening()
{
	// Start listening on the socket
	listening = listen(getSocketFd(), _backlog);
}