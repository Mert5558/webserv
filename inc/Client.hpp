#pragma once

#include "Webserv.hpp"
#include "ParseHttp.hpp"

enum class ClientState	{IDLE, HEADERS_RECEIVED, BODY_RECEIVED, COMPLETE, ERROR};

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

		ClientState		state;
		HttpRequest     request;


		Client() : fd(-1), expected_len(0), received_len(0), isComplete(false), header_received(false), body_received(false), body_start(0) {}
		Client(int _fd) : fd(_fd), expected_len(0), received_len(0), isComplete(false), header_received(false), body_received(false), body_start(0) {}
		~Client() {}


		void reset()
		{
			recv_buffer.clear();
			header_str.clear();
			expected_len = 0;
			body_start = 0;
			disconnect = false;
			state = ClientState::IDLE;
			request.reset();
		}
};