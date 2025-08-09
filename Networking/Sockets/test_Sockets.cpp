#include "ListenSocket.hpp"
#include "BindSocket.hpp"

int main()
{
	std::cout << "Socket Programming Example" << std::endl;
	std::cout << "Binding socket..." << std::endl;
	bindSocket bs = bindSocket(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY);
	std::cout << "Listening on socket..." << std::endl;
	listenSocket ls = listenSocket(AF_INET, SOCK_STREAM, 0, 8081, INADDR_ANY, 10);
	std::cout << "Socket is ready to accept connections." << std::endl;
}