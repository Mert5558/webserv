#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Webserv.hpp"
#include "ParseHttp.hpp"
#include "httpResponse.hpp"

enum class ClientState { IDLE, COMPLETE, ERROR };

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