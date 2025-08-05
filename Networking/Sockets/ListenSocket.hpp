/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   listenSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 21:49:56 by cmakario          #+#    #+#             */
/*   Updated: 2025/08/06 00:31:23 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LISTEN_SOCKET_HPP
#define LISTEN_SOCKET_HPP

#include "BindSocket.hpp"

class listenSocket : public bindSocket {
private:
	int _backlog;
	int listening;
public:
	// Constructor
	listenSocket(int domain, int service, int protocol, int port, u_long interface, int backlog);

	// Connect through listen
	void startlinstening();
};


#endif // !LISTEN_SOCKET_HPP
