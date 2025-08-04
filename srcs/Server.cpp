#include "../inc/Server.hpp"
#include "../inc/Webserv.hpp"
#include "../inc/ParseConfig.hpp"
#include "../inc/ParseHttp.hpp"

Server::Server()
{}

Server::~Server()
{}

void Server::startServer(ParseConfig parse)
{
	HttpRequest request;
	std::vector<pollfd> fds;

	std::vector<InitConfig> &servers = parse.getServers();
	
	serverSetup(servers);

	fds = initPollfd(servers);
	
	while (true)
	{
		
	}

}

std::vector<pollfd> Server::initPollfd(std::vector<InitConfig> &servers)
{
	std::vector<pollfd> fds;
	
	for (size_t i = 0; i < servers.size(); i++)
	{
		pollfd pfd;

		pfd.fd = servers[i].getFd();
		pfd.events = POLL_IN;
		fds.push_back(pfd);

		if (fcntl(pfd.fd, F_SETFL, O_NONBLOCK) == -1)
			std::cout << "Error: setting fd to nonblock" << std::endl;
	}
	return (fds);
}

void Server::parseHttp(std::vector<InitConfig> &servers, HttpRequest &request)
{
	struct pollfd fds[1];
	std::cout << servers[0].getFd() << " before ---------------------" << std::endl;
	fds[0].fd = servers[0].getFd();
	std::cout << servers[0].getFd() << " after ---------------------" << std::endl;
	fds[0].events = POLLIN;

	while (true)
	{
		int ret = poll(fds, 1, -1);
		if (ret < 0)
		{
			perror("poll");
			break;
		}

		if (fds[0].revents & POLLIN)
		{
			// 6. Accept a new connection
			struct sockaddr_in client_addr;
			socklen_t client_len = sizeof(client_addr);
			int client_fd = accept(servers[0].getFd(), (struct sockaddr*)&client_addr, &client_len);
			if (client_fd < 0)
			{
				perror("accept");
				continue;
			}

			std::cout << "\n" << "Client connected: " << inet_ntoa(client_addr.sin_addr) << std::endl;

			// Handle multiple requests from the same client
			while (true)
			{
				std::string buffer = request.receiveRequest(client_fd);
				request.parseRequest(buffer);
				// request.isValidMethod(); // maybe we should encaptulate the parseRequest() function iside this one and rename it
				
				
				// 8. Send a basic HTTP response with keep-alive
				std::string response = request.buildResponse();
				
				int bytes_sent = send(client_fd, response.c_str(), response.size(), 0);
				if (bytes_sent < 0)
				{
					perror("send");
					break;
				}

				std::cout << "Response sent (" << bytes_sent << " bytes)." << std::endl;
			}
			close(client_fd); // Close the connection when the client disconnects
		}
		close(servers[0].getFd());
	}
}

void Server::serverSetup(std::vector<InitConfig> &servers)
{
	std::cout << "servers being initialized..." << std::endl;     //logger

	for (size_t i = 0; i < servers.size(); i++)
	{
		bool duplicate = false;

		for (size_t j = 0; j < i; j++)
		{
			if (servers[j].getHost() == servers[i].getHost() &&
				servers[j].getPort() == servers[i].getPort())
			{
				throw ConfigError("Error: do not support virtual host!");
				duplicate = true;
				break;
			}
		}

		if (!duplicate)
		{
			if (!servers[i].createAndBindSocket())
				throw ConfigError("Failed to setup server socket!");
		}
		std::cout << "Server created 'host: ... ', port: '...'" << std::endl;     //logger
	}
}
