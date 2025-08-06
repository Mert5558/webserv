#include "../inc/Server.hpp"
#include "../inc/ParseConfig.hpp"
#include "../inc/ParseHttp.hpp"

Server::Server()
{}

Server::~Server()
{}

void Server::startServer(ParseConfig parse)
{
	HttpRequest request;

	std::vector<InitConfig> &servers = parse.getServers();
	
	serverSetup(servers);
	parseHttp(servers, request);
	// while (true)
	// 	sleep(1);
}

void Server::parseHttp(std::vector<InitConfig> &servers, HttpRequest &request)
{
	struct pollfd fds[1];
	fds[0].fd = servers[0].getFd();
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
				std::string buffer;
				std::cout << "buffer size at the begin: " << buffer.size() << std::endl;
				ssize_t bytes = request.receive(client_fd, buffer);
				std::cout << "buffer size after: " << buffer.size() << std::endl;
				if (bytes <= 0)
				{
					break;
				}
				
				// Try to parse as much as possible from 'buffer'
				if (request.parseRequest(buffer))
				{
					// Request complete
					std::string response = request.buildResponse();
					send(client_fd, response.c_str(), response.size(), 0);
					
					// print the response
					std::cout << "\nResponse sent (" << response.size() << " bytes).\n" << std::endl;
					// std::cout << response.c_str() << std::endl;
					
					buffer.clear();
					request.reset();
				}
				else
				{
					// Not complete yet → keep reading
					continue;
				}		
			}
		}
		// close(client_fd); // Close the connection when done
		// close(servers[0].getFd());
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
			std::cout << "-----------------------------------------------" << std::endl;
			if (!servers[i].createAndBindSocket())
				throw ConfigError("Failed to setup server socket!");
		}
		std::cout << servers[0].getFd() << " after socket creation" << std::endl;
		std::cout << "Server created 'host: ... ', port: '...'" << std::endl;     //logger
	}
}
