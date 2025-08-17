#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Webserv.hpp"
#include "ParseHttp.hpp"
#include "HttpResponse.hpp"

enum class ClientState { IDLE, HEADERS_RECEIVED, BODY_RECEIVED, COMPLETE, ERROR };

class Client
{
	public:
		int				fd;
		int				server_index;
		ClientState		state;
		HttpRequest		request;
		HttpResponse	response;

		// Additions for non-blocking writes:
		std::string		outBuf;		// pending response bytes to send
		size_t			outOff;		// how many bytes already sent

		Client() : fd(-1),server_index(-1), state(ClientState::IDLE), outOff(0) {}

		Client(int _fd, int _server_index) : fd(_fd), server_index(_server_index), state(ClientState::IDLE), outOff(0) {}

		~Client() {}
};

#endif // !CLIENT_HPP
