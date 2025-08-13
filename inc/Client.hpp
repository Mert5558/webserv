#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Webserv.hpp"
#include "ParseHttp.hpp"

enum class ClientState { IDLE, COMPLETE, ERROR };

class Client
{
public:
	int				fd;
	std::string		recv_buffer;
	std::string		send_buffer;
	size_t			send_offset;
	bool			disconnect;
	ClientState		state;
	HttpRequest		request;

	// Constructors and Destructor
	Client() : fd(-1), send_offset(0), disconnect(false), state(ClientState::IDLE) {}
	Client(int _fd) : fd(_fd), send_offset(0), disconnect(false), state(ClientState::IDLE) {}
	~Client() {}

	// Function to reset the client state
	void reset()
	{
		recv_buffer.clear();
		send_buffer.clear();
		send_offset = 0;
		disconnect = false;
		state = ClientState::IDLE;
		request.reset();
	}

	// Function to make a simple response
	void makeSimpleResponse(int code, const std::string &text, const std::string &body)
	{
		std::ostringstream ss;
		ss << "HTTP/1.1 " << code << " " << text << "\r\n";
		ss << "Content-Type: text/plain\r\n";
		ss << "Content-Length: " << body.size() << "\r\n";
		ss << "Connection: close\r\n\r\n";
		ss << body;
		send_buffer = ss.str();
		send_offset = 0;
	}
};

#endif // !CLIENT_HPP