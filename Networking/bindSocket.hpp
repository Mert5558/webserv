/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bindSocket.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 19:43:22 by cmakario          #+#    #+#             */
/*   Updated: 2025/08/05 20:07:41 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BIND_SOCKET_HPP
#define BIND_SOCKET_HPP

#include "basicSocket.hpp"

class bindSocket : public basicSocket {
public:
	bindSocket(int domain, int service, int protocol, int port, u_long interface);
	int connectTheSocket(int sock, struct sockaddr_in address) const override;
	~bindSocket();
	void bindToAddress(const std::string& address, int port);
	
private:
	int socket_fd;
};

#endif