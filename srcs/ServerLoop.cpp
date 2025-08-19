#include "../inc/ServerLoop.hpp"
#include "../inc/Webserv.hpp"
#include "../inc/ParseConfig.hpp"
#include "../inc/ParseHttp.hpp"
#include "../inc/httpResponse.hpp"
#include "../inc/Client.hpp"

ServerLoop::ServerLoop()
{}

ServerLoop::~ServerLoop()
{}

void ServerLoop::startServer(ParseConfig parse)
{
	std::vector<InitConfig> &servers = parse.getServers();

	serverSetup(servers);

	initPollfd(servers);

	while (true)
	{
		int ready = poll(fds.data(), fds.size(), 1000);
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

				if (isSocketFd(fds[i].fd, servers))
				{
					acceptClient(fds[i].fd);
					std::cout << "new client connected: " << std::endl;
				}
				else
				{
					std::cout << "this is a request for a client, here we read, parse and then send the response" << std::endl;

					int client_fd = fds[i].fd;

					bool done = clients[client_fd].request.receiveReq(client_fd);

					if (done)
					{
						std::cout << "entered done loop" << std::endl;
						if (!clients[client_fd].request.disconnect)
						{
							clients[client_fd].request.parseRequestFromCompleteBuffer();

							std::string responseStr = clients[client_fd].response.buildResponse(clients[client_fd].request);
							fds[i].revents = POLLOUT;
							if (fds[i].revents & POLLOUT)
							{
								int bytes_sent = send(client_fd, responseStr.c_str(), responseStr.size(), 0);
								if (bytes_sent < 0)
								{
									perror("send");
									break;
								}
								std::cout << "response send to: " << fds[i].fd << std::endl;
								removeClient(fds[i].fd);
							}
						}
					}
				}
			}
		}
		updateFds();
	}
}

bool ServerLoop::isSocketFd(int client_fd, std::vector<InitConfig> &servers)
{
	for (size_t s = 0; s < servers.size(); ++s)
	{
		if (client_fd == servers[s].getFd())
		{
			return (true);
		}
	}
	return (false);
}

void ServerLoop::acceptClient(int client_fd)
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int cfd = accept(client_fd, (struct sockaddr*)&client_addr, &client_len);
	if (cfd == 0)
		std::cout << "Error: accept client" << std::endl;
	
	int flags = fcntl(cfd, F_GETFL, 0);
	if (flags == -1)
		std::cout << "Error: fcntl F_GETFL" << std::endl;
	else if (fcntl(cfd, F_SETFL, flags | O_NONBLOCK) == -1)
		std::cout << "Error: fcntl F_SETFL O_NONBLOCK" << std::endl;

	clients[cfd] = Client(cfd);

	struct pollfd new_pollfd;
	new_pollfd.fd = cfd;
	new_pollfd.events = POLLIN;
	toAdd.push_back(new_pollfd);
}

void ServerLoop::removeClient(int client_fd)
{
    std::cout << "client removed: " << client_fd << std::endl;
	toRemove.push_back(client_fd);
	clients.erase(client_fd);
	close(client_fd);
}

void ServerLoop::updateFds()
{
	for (size_t i = 0; i < toRemove.size(); i++)
	{
		for (size_t j = 0; j < fds.size(); j++)
		{
			if (fds[j].fd == toRemove[i])
			{
				fds.erase(fds.begin() + j);
				break;
			}
		}
	}

	for (size_t x = 0; x < toAdd.size(); x++)
	{
		fds.push_back(toAdd[x]);
	}

	toAdd.clear();
	toRemove.clear();
}


void ServerLoop::initPollfd(std::vector<InitConfig> &servers)
{
	for (size_t i = 0; i < servers.size(); i++)
	{
		pollfd pfd;

		pfd.fd = servers[i].getFd();
		pfd.events = POLLIN;
		fds.push_back(pfd);

		if (fcntl(pfd.fd, F_SETFL, O_NONBLOCK) == -1)
			std::cout << "Error: setting fd to nonblock" << std::endl;
	}
}

void ServerLoop::serverSetup(std::vector<InitConfig> &servers)
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
		// std::cout << servers[0].getFd() << " after socket creation" << std::endl;
		// std::cout << "Server created 'host: ... ', port: '...'" << std::endl;     //logger
		std::cout << "Server socket created and bound successfully: "
          << "Host: " << servers[i].getHost()
          << ", Port: " << servers[i].getPort()
          << ", Fd: " << servers[i].getFd()
          << std::endl;
		std::cout << "Server created 'host: ... ', port: '...'" << std::endl;     //logger
	}
}
