#include "../inc/ServerLoop.hpp"
#include "../inc/Webserv.hpp"
#include "../inc/ParseConfig.hpp"
#include "../inc/HttpRequest.hpp"
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

					clients[client_fd] = Client(client_fd);

					std::cout << "new client connected: " << client_fd << std::endl;
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

							int bytes_sent = send(client_fd, responseStr.c_str(), responseStr.size(), 0);
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
	}
}

void ServerLoop::removeFd(std::vector<pollfd> &fds, size_t index)
{
    if (fds[index].fd >= 0)
    {
        std::cout << "Closing fd: " << fds[index].fd << std::endl;
        close(fds[index].fd);
    }
    fds.erase(fds.begin() + index);
    std::cout << "Removed fd at index: " << index << std::endl;
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
