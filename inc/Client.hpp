#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Webserv.hpp"
#include "ParseHttp.hpp"
#include "httpResponse.hpp"

enum class ClientState { IDLE, HEADERS_RECEIVED, BODY_RECEIVED, COMPLETE, ERROR };

class Client
{
	public:
		int				fd;
		ClientState		state;
		HttpRequest		request;
		httpResponse	response;

		// Additions for non-blocking writes:
		std::string		outBuf;		// pending response bytes to send
		size_t			outOff;		// how many bytes already sent

		Client() : fd(-1), state(ClientState::IDLE), outOff(0) {}

		Client(int _fd) : fd(_fd), state(ClientState::IDLE), outOff(0) {}

		~Client() {}
};

#endif // !CLIENT_HPP
