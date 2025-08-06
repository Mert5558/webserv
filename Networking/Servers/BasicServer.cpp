/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BasicServer.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/06 01:08:14 by cmakario          #+#    #+#             */
/*   Updated: 2025/08/06 02:05:51 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BasicServer.hpp"

// Constructor
basicServer::basicServer(int domain, int service, int protocol, int port, u_long interface, int backlog)
{
	socket = new listenSocket(domain, service, protocol, port, interface, backlog);
}

// Getters
listenSocket* basicServer::getSocket() const
{
	return socket;
}