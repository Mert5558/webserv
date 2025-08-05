/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bindSocket.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 19:43:22 by cmakario          #+#    #+#             */
/*   Updated: 2025/08/05 22:22:18 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BIND_SOCKET_HPP
#define BIND_SOCKET_HPP

#include "BasicSocket.hpp"

class bindSocket : public basicSocket {
public:
	// Constructor
	bindSocket(int domain, int service, int protocol, int port, u_long interface);
	// Connect through bind
	int connectTheSocket(int sock, struct sockaddr_in address) const override;
	// Bind the socket to a specific address and port
	void bindToAddress(const std::string& address, int port);
};

#endif