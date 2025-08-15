#pragma once

#include "Webserv.hpp"
#include "HttpRequest.hpp"
#include "httpResponse.hpp"

enum class ClientState	{IDLE, HEADERS_RECEIVED, BODY_RECEIVED, COMPLETE, ERROR};

class Client
{
	public:
		int				fd;

		ClientState		state;
		HttpRequest     request;
		httpResponse 	response;


		Client() : fd(-1) {}
		Client(int _fd) : fd(_fd) {}
		~Client() {}

};