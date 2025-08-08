#include "../inc/Server.hpp"
#include "../inc/Webserv.hpp"
#include "../inc/ParseConfig.hpp"
#include "../inc/ParseHttp.hpp"
#include "../inc/httpResponse.hpp"

Server::Server()
{}

Server::~Server()
{}

void Server::startServer(ParseConfig parse)
{
	HttpRequest request;
	httpResponse response;
	std::vector<pollfd> fds;

	std::vector<InitConfig> &servers = parse.getServers();
	
	serverSetup(servers);

	fds = initPollfd(servers);
	
	while (true)
	{
		int ready = poll(fds.data(), fds.size(), 10);
		if (ready < 0)
			std::cout << "Error poll()!" << std::endl;
		
		for (size_t i = 0; i < fds.size(); i++)
		{
			if (fds[i].revents & POLLIN)
			{
				
				for (size_t k = 0; k < fds.size(); k++)
				{
					std::cout << std::endl;
					std::cout << "fd for index: " << k << fds[k].fd << std::endl;
					std::cout << "event for index: " << k << fds[k].events << std::endl;
					std::cout << "revent for index: " << k << fds[k].revents << std::endl;
					std::cout << std::endl;
				}


				bool isSocketFd = false;
				for (size_t s = 0; s < servers.size(); ++s)
				{
					if (fds[i].fd == servers[s].getFd())
					{
						isSocketFd = true;
						break;
					}
				}

				if (isSocketFd)
				{
					std::cout << "we accept the new client" << std::endl;
					struct sockaddr_in client_addr;
					socklen_t client_len = sizeof(client_addr);
	
					int client_fd = accept(fds[i].fd, (struct sockaddr*)&client_addr, &client_len);
					if (client_fd < 0)
						std::cout << "Error client_fd" << std::endl;
					
					struct pollfd new_pollfd;
					new_pollfd.fd = client_fd;
					new_pollfd.events = POLLIN;
					fds.push_back(new_pollfd);
	
					std::cout << "new client connected: " << client_fd << std::endl;
				}
				else
				{
					std::cout << "this is a request for a client, here we read, parse and then send the response" << std::endl;

					std::string buffer = request.receiveRequest(fds[i].fd);
					request.parseRequest(buffer);
					// request.isValidMethod(); // maybe we should encaptulate the parseRequest() function iside this one and rename it
					
					
					// 8. Send a basic HTTP response with keep-alive
					std::string respons = response.buildResponse();

					int bytes_sent = send(fds[i].fd, respons.c_str(), respons.size(), 0);
					if (bytes_sent < 0)
					{
						perror("send");
						break;
					}

					std::cout << "response send to: " << fds[i].fd << std::endl;

					std::cout << "fd removed: " << fds[i].fd << std::endl;
					close(fds[i].fd);
					fds.erase(fds.begin() + i);
					i--;

				}
			}
		}
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

void Server::parseHttp(std::vector<InitConfig> &servers, HttpRequest &request, httpResponse &response)
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
				std::string raw_response = response.buildResponse();
				
				int bytes_sent = send(client_fd, raw_response.c_str(), raw_response.size(), 0);
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


//============POLL EXAMPLE============
// enum Flags
// {
// 	FLAG_POLLERR    = (0b1),
// 	FLAG_POLLHUP   = (0b10),
// 	FLAG_POLLIN   = (0b100),
// 	FLAG_POLLOUT = (0b1000),
// };

// void whatever()
// {
// 	int pollfds_count = 0;
// 	pollfd pollfds[16] = {};

// 	pollfds[0].fd = server_fd;
// 	pollfds[0].events = POLLIN;
// 	pollfds_count += 1;

// 	poll(pollfds, pollfds_count, 1000);
// 	if (pollfds[0].revents & POLLIN)
// 	{

// 	}
// }
