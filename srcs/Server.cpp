#include "../inc/Server.hpp"
#include "../inc/ParseConfig.hpp"
#include "../inc/ParseHttp.hpp"

Server::Server()
{}

Server::~Server()
{}

// void Server::startServer(ParseConfig parse)
// {
// 	HttpRequest request;

// 	std::vector<InitConfig> &servers = parse.getServers();
	
// 	serverSetup(servers);
// 	parseHttp(servers, request);
// 	// while (true)
// 	// 	sleep(1);
// }


void Server::startServer(ParseConfig parse)
{
	HttpRequest request;
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
				if (fds[i].fd == -1)
				{
					std::cout << "Skipping invalid fd: " << fds[i].fd << std::endl;
					continue;
				}

				// Print the current state of pollfd
				std::cout << "DEBUG: Current state of pollfd array (size: " << fds.size() << "):" << std::endl;
				for (size_t k = 0; k < fds.size(); k++)
				{
					std::cout << "  [Index " << k << "] fd: " << fds[k].fd
							<< ", events: " << fds[k].events
							<< ", revents: " << fds[k].revents << std::endl;
				}
				std::cout << "-----------------------------------------------" << std::endl;


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
					
					// Add new client to poll array
					struct pollfd new_pollfd;
					new_pollfd.fd = client_fd;
					new_pollfd.events = POLLIN;
					fds.push_back(new_pollfd);
	
					std::cout << "new client connected fd: " << client_fd << std::endl;
				}
				else
				{
					std::cout << "this is a request for a client, here we read, parse and then send the response" << std::endl;
					std::string buffer;
					while (true)
					{
						ssize_t bytes = request.receive(fds[i].fd, buffer);
						if (bytes <= 0)
						{
							break;
						}
						
						ParseResult result = request.parseRequestPartial(buffer);
						
						switch (result)
						{
							case ParseResult::INCOMPLETE:
								continue;
							
							case ParseResult::COMPLETE:
							{
								std::string response = request.buildResponse();
								send(fds[i].fd, response.c_str(), response.size(), 0);

								request.log_first_line();  
								request.log_headers();
								
								// print the response
								// std::cout << "\nResponse sent (" << response.size() << " bytes).\n" << std::endl;
								// std::cout << response.c_str() << std::endl;
								
								request.reset();
								buffer.clear();
								break;
								
							}
							
							case ParseResult::ERROR:
							{
								std::string error = request.buildResponse();
								send(fds[i].fd, error.c_str(), error.size(), 0);
								request.reset();
								buffer.clear();
								break;
							}
							
						}

						std::cout << "response send to: " << fds[i].fd << std::endl;
						removeFd(fds, i);

					}
					// close(servers[0].getFd());

				}
			}
		}
	}
}


void Server::removeFd(std::vector<pollfd> &fds, size_t index)
{
    if (fds[index].fd >= 0)
    {
        std::cout << "Closing fd: " << fds[index].fd << std::endl;
        close(fds[index].fd);
    }
    fds.erase(fds.begin() + index);
    std::cout << "Removed fd at index: " << index << std::endl;
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


// void Server::parseHttp(std::vector<InitConfig> &servers, HttpRequest &request)
// {
// 	struct pollfd fds[1];
// 	fds[0].fd = servers[0].getFd();
// 	fds[0].events = POLLIN;
	
	
// 	while (true)
// 	{
// 		int ret = poll(fds, 1, -1);
// 		if (ret < 0)
// 		{
// 			perror("poll");
// 			break;
// 		}
		
// 		if (fds[0].revents & POLLIN)
// 		{
// 			// 6. Accept a new connection
// 			struct sockaddr_in client_addr;
// 			socklen_t client_len = sizeof(client_addr);
// 			int client_fd = accept(servers[0].getFd(), (struct sockaddr*)&client_addr, &client_len);
// 			if (client_fd < 0)
// 			{
// 				perror("accept");
// 				continue;
// 			}
			
// 			std::cout << "\n" << "Client connected: " << inet_ntoa(client_addr.sin_addr) << std::endl;
			
// 			// Handle multiple requests from the same client
// 			std::string buffer;
// 			while (true)
// 			{
// 				ssize_t bytes = request.receive(client_fd, buffer);
// 				if (bytes <= 0) break;
				
// 				ParseResult result = request.parseRequestPartial(buffer);
				
// 				switch (result)
// 				{
// 					case ParseResult::INCOMPLETE:
// 						continue;
					
// 					case ParseResult::COMPLETE:
// 					{
// 						std::string response = request.buildResponse();
// 						send(client_fd, response.c_str(), response.size(), 0);

// 						request.log_first_line();  
// 						request.log_headers();
						
// 						// print the response
// 						// std::cout << "\nResponse sent (" << response.size() << " bytes).\n" << std::endl;
// 						// std::cout << response.c_str() << std::endl;
						
// 						request.reset();
// 						buffer.clear();
// 						break;
						
// 					}
					
// 					case ParseResult::ERROR:
// 					{
// 						std::string error = request.buildResponse();
// 						send(client_fd, error.c_str(), error.size(), 0);
// 						request.reset();
// 						buffer.clear();
// 						break;
// 					}
					
// 				}
// 				close(client_fd); // Close the connection when done
						
// 			}
// 			close(servers[0].getFd());
// 		}
// 	}
// }

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
		// std::cout << servers[0].getFd() << " after socket creation" << std::endl;
		// std::cout << "Server created 'host: ... ', port: '...'" << std::endl;     //logger
		std::cout << "Server socket created and bound successfully: "
          << "Host: " << servers[i].getHost()
          << ", Port: " << servers[i].getPort()
          << ", Fd: " << servers[i].getFd()
          << std::endl;
	}
}
