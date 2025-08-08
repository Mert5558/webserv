#include "../inc/Server.hpp"
#include "../inc/Webserv.hpp"
#include "../inc/ParseConfig.hpp"
#include "../inc/ParseHttp.hpp"
#include "../inc/Client.hpp"

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

	std::unordered_map<int, Client> clients;

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

					bool done = receiveReq(client_fd, clients);

					if (done)
					{
						std::cout << "entered done loop" << std::endl;
						if (!clients[client_fd].disconnect)
						{
							
							request.parseRequest(clients[client_fd].recv_buffer);

							std::string response = request.buildResponse();

							int bytes_sent = send(client_fd, response.c_str(), response.size(), 0);
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

bool Server::receiveReq(int client_fd, std::unordered_map<int, Client> &clients)
{
	Client &client = clients[client_fd];
	char buf[4096];
	ssize_t bytes = recv(client_fd, buf, sizeof(buf), 0);
	if (bytes <= 0)
	{
		client.disconnect = true;
		return (true);
	}

	client.recv_buffer.append(buf, bytes);

	if (!client.header_received)
	{
		client.header_received = true;
		size_t header_end = client.recv_buffer.find("\r\n\r\n");

		if (header_end != std::string::npos)
		{
			client.header_str = client.recv_buffer.substr(0, header_end + 4);

			size_t cl_pos = client.header_str.find("Content-Length:");
			if (cl_pos != std::string::npos)
			{
				size_t value_start = client.header_str.find_first_not_of(" ", cl_pos + 15);
				size_t value_end = client.header_str.find("\r\n", value_start);
				std::string str_len = client.header_str.substr(value_start, value_end - value_start);
				client.expected_len = std::atoi(str_len.c_str());
			}
			else
				client.expected_len = 0;
			
			client.body_start = header_end + 4;
		}
	}

	if (client.header_received && !client.body_received)
	{
		size_t total_body_size = client.recv_buffer.size() - client.body_start;
		if (client.expected_len == 0 || total_body_size >= client.expected_len)
		{
			client.body_received = true;
			client.isComplete = true;
		}
	}

	return (client.isComplete);
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
