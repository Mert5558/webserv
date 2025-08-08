#pragma once

#include "Webserv.hpp"

class Client
{
	public:
		int				fd;
		std::string		recv_buffer;
		size_t 			expected_len;
		size_t 			received_len;
		bool 			isComplete;
		bool 			disconnect;
		bool 			header_received;
		bool 			body_received;
		std::string		header_str;
		size_t			body_start;


		Client() : fd(-1), expected_len(0), received_len(0), isComplete(false), header_received(false), body_received(false), body_start(0) {}
		Client(int _fd) : fd(_fd), expected_len(0), received_len(0), isComplete(false), header_received(false), body_received(false), body_start(0) {}
		~Client() {}
};