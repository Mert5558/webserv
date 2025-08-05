/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   basicSocket.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 15:53:55 by cmakario          #+#    #+#             */
/*   Updated: 2025/08/05 16:28:23 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BASIC_SOCKET_HPP
#define BASIC_SOCKET_HPP

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

class basicSocket {
private:
		struct sockaddr_in address;
		int socket_fd;
public:
		// Constructor
		basicSocket(int domain, int service, int protocol, int port, u_long interface);
};

#endif // BASIC_SOCKET_HPP