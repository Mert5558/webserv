#include "../inc/ServerLoop.hpp"
#include "../inc/Webserv.hpp"
#include "../inc/ParseConfig.hpp"
#include "../inc/HttpRequest.hpp"
#include "../inc/httpResponse.hpp"
#include "../inc/Client.hpp"



// Set a file descriptor to non-blocking mode.
static bool setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) return false;
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) return false;
	return true;
}

// Check if fd matches any server fd.
static int findServerIndexByFd(const std::vector<InitConfig> &servers, int fd)
{
	for (size_t i = 0; i < servers.size(); ++i)
	{
		if (servers[i].getFd() == fd)
			return static_cast<int>(i);
	}
	return -1;
}

// Constructor and Destructor
ServerLoop::ServerLoop()
{}

ServerLoop::~ServerLoop()
{}
// Setup each server socket: create, bind, listen, and set non-blocking.
void ServerLoop::serverSetup(std::vector<InitConfig> &servers)
{
	std::cout << "servers being initialized..." << std::endl;     //logger
	
	// Create/bind/listen each server and set non-blocking
	for (size_t i = 0; i < servers.size(); ++i)
	{
		if (!servers[i].createAndBindSocket())
			throw ConfigError("Failed to setup server socket!");

		int sfd = servers[i].getFd();
		if (!setNonBlocking(sfd))
			throw ConfigError("Failed to set listen socket non-blocking!");

		if (listen(sfd, 128) < 0)
			throw ConfigError("listen() failed");

		std::cout << "Server socket ready: host=" << servers[i].getHost()
			<< " port=" << servers[i].getPort()
			<< " fd=" << servers[i].getFd() << std::endl;
	}
}

// Initialize pollfd structures for each server socket.
void ServerLoop::initPollfd(std::vector<InitConfig> &servers)
{
	fds.clear();
	// Register all server fds for POLLIN
	for (size_t i = 0; i < servers.size(); ++i)
	{
		struct pollfd p;
		p.fd = servers[i].getFd();
		p.events = POLLIN;
		p.revents = 0;
		fds.push_back(p);
	}
}

// Remove a file descriptor from the pollfd vector and close it.
void ServerLoop::removeFd(std::vector<pollfd> &fds, size_t index)
{
	if (index >= fds.size())
		return; // Prevent out-of-bounds access

	if (fds[index].fd >= 0)
	{
		std::cout << "Closing fd: " << fds[index].fd << std::endl;
		close(fds[index].fd);
	}

	int fd = fds[index].fd;
	fds.erase(fds.begin() + index);
	std::cout << "(-) Removed fd=" << fd << " at index: " << index << std::endl;

}
// ================

void ServerLoop::startServer(ParseConfig parse)
{
	std::vector<InitConfig> &servers = parse.getServers();

	if (servers.empty())
		throw ConfigError("No servers configured!");

	std::cout << "[BOOT] serverSetup()..." << std::endl;
	serverSetup(servers);

	std::cout << "[BOOT] initPollfd()..." << std::endl;
	initPollfd(servers);

	std::cout << "[READY] Poll vector size: " << fds.size() << " (listening sockets added)" << std::endl;

	while (true)
	{
		int ret = poll(&fds[0], fds.size(), 2000); // 2s timeout
		if (ret < 0)
		{
			if (errno == EINTR)
			{
				std::cout << "[POLL] interrupted by signal, continuing..." << std::endl;
				continue;
			}
			perror("poll");
			break;
		}
		if (ret == 0)
		{
			// Timeout: nothing happened
			std::cout << "[POLL] timeout (no events). fds.size()=" << fds.size() << std::endl;
			dumpTopology(servers);
			continue;
		}

		for (size_t i = 0; i < fds.size(); /* manual ++ inside */)
		{
			pollfd &pfd = fds[i];

			// Handle errors first

			// if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
			// {
			// 	if (clients.count(pfd.fd))
			// 	{
			// 		close(pfd.fd);
			// 		clients.erase(pfd.fd);
			// 	}
			// 	removeFd(fds, i);
			// 	continue; // don't ++i, because we erased current
			// }
			if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				std::cout << "[EVT] fd=" << pfd.fd << " has error/hup/nval (revents=" << pfd.revents << "), closing." << std::endl;
				if (clients.count(pfd.fd))
				{
					std::cout << "\t[CLIENT] removing client for fd=" << pfd.fd << std::endl;
					close(pfd.fd);
					clients.erase(pfd.fd);
				}
				std::cout << "\t[FD] erase fd=" << pfd.fd << " at index " << i << std::endl;
				removeFd(fds, i); // no i++ here
				continue;
			}

			// If it's a listen socket → accept
			int serverIdx = findServerIndexByFd(servers, pfd.fd);
			if (serverIdx >= 0)
			{
				if (pfd.revents & POLLIN)
				{
					std::cout << "[ACCEPT] ready on listen fd=" << pfd.fd
							  << " (server index " << serverIdx << "), accepting..." << std::endl;
					// Accept as many as possible (non-blocking accept)
					while (true)
					{
						int cfd = accept(pfd.fd, NULL, NULL);
						if (cfd < 0)
						{
							if (errno == EAGAIN || errno == EWOULDBLOCK)
							{
								// No more clients to accept right now
								std::cout << "[ACCEPT] no more clients to accept on fd=" << pfd.fd << std::endl;
								break; // no more to accept
							}
							perror("accept");
							std::cout << "\t[ACCEPT] error during accept(); stopping accept loop." << std::endl;
							break;
						}
						if (!setNonBlocking(cfd))
						{
							std::cout << "\t[ACCEPT] setNonBlocking failed for fd=" << cfd << ", closing." << std::endl;
							close(cfd);
							continue;
						}

						// Register client fd for reading
						struct pollfd cp;
						cp.fd = cfd;
						cp.events = POLLIN;
						cp.revents = 0;
						fds.push_back(cp);

						clients[cfd] = Client(cfd, serverIdx);
						std::cout << "New client connected: fd=" << cfd << std::endl;
					}
				}
				++i;
				continue;
			}

			// It's a client fd
			if (pfd.revents & POLLIN)
			{
				std::cout << "[READ] POLLIN on client fd=" << pfd.fd << std::endl;

				// Read as much as we can (HttpRequest handles state)
				Client &cl = clients[pfd.fd];
				bool ok = cl.request.receiveReq(pfd.fd);
				std::cout << "\t[READ] receiveReq() returned " << (ok ? "true" : "false") << std::endl;

				if (!ok || cl.request.disconnect)
				{
					std::cout << "\t[READ] Message was not received either client requested disconnect, closing fd=" << pfd.fd << std::endl;
					close(pfd.fd);
					clients.erase(pfd.fd);
					removeFd(fds, i);
					continue;
				}
				// if (!ok)
				// {
				// 	close(pfd.fd);
				// 	clients.erase(pfd.fd);
				// 	removeFd(fds, i);
				// 	continue;
				// }

				if (cl.request.isComplete)
				{
					std::cout << "\t[READ] Request complete for fd=" << pfd.fd << std::endl;

					// TODO: Build response here (static handler for now)
					parseHttp(servers, cl.request, cl.response);

					// Serialize response into outBuf for non-blocking write
					cl.outBuf = cl.response.buildResponse();
					cl.outOff = 0;

					// Switch this fd to POLLOUT
					pfd.events = POLLOUT;
					std::cout << "\t[WRITE-ARM] fd=" << pfd.fd << " switched to POLLOUT, bytes to send=" << cl.outBuf.size() << std::endl;
				}

				++i;
				continue;
			}

			// Handle POLLOUT events - write response
			if (pfd.revents & POLLOUT)
			{
				// Non-blocking partial send
				Client &cl = clients[pfd.fd];
				size_t left = cl.outBuf.size() - cl.outOff;

				std::cout << "[WRITE] POLLOUT on fd=" << pfd.fd
						  << " (left=" << left << " bytes)" << std::endl;

				const char *data = cl.outBuf.c_str() + cl.outOff;

				ssize_t n = send(pfd.fd, data, left, 0);

				if (n < 0)
				{
					if (errno == EAGAIN || errno == EWOULDBLOCK)
					{
						// try later
						std::cout << "\t[WRITE] EAGAIN/EWOULDBLOCK, will retry later." << std::endl;
						++i;
						continue;
					}

					// real error
					perror("send");
					std::cout << "\t[WRITE] hard send() error, closing fd=" << pfd.fd << std::endl;
					close(pfd.fd);
					clients.erase(pfd.fd);
					std::cout << "\t[FD] erase fd=" << pfd.fd << " at index " << i << std::endl;
					removeFd(fds, i);
					continue;
				}

				cl.outOff += static_cast<size_t>(n);
				std::cout << "\t[WRITE] sent " << n << " bytes (off=" << cl.outOff << "/" << cl.outBuf.size() << ")" << std::endl;

				if (cl.outOff >= cl.outBuf.size())
				{
					// Done writing. Close connection .
					std::cout << "\t[DONE] response fully sent on fd=" << pfd.fd << " -> closing connection" << std::endl;
					close(pfd.fd);
					clients.erase(pfd.fd);
					std::cout << "\t[FD] erase fd=" << pfd.fd << " at index " << i << std::endl;
					removeFd(fds, i);
					continue;
				}

				++i;
				continue;
			}
			if (pfd.revents == 0)
			{
				// No events to handle, just continue
				std::cout << "[POLL] no events for fd=" << pfd.fd << " (revents=0), continuing..." << std::endl;
			}
			// Move on
			++i;
		}
		dumpTopology(servers);
	}
}


void ServerLoop::parseHttp(std::vector<InitConfig> &servers, HttpRequest &request, httpResponse &response)
{
	// simple routing: choose the first server
	// pick servers[0] to get root/index. Extend later to pick by Host.
	InitConfig *srv = servers.empty() ? NULL : &servers[0];

	// Prepare a static file response under root based on request target
	response.prepare(request, srv);
}

// Helper function to dump the current server topology and client states
void ServerLoop::dumpTopology(const std::vector<InitConfig> &servers)
{
	std::ostringstream oss;
	oss << "[TOPO] ";

	for (size_t i = 0; i < servers.size(); ++i)
	{
		oss << "S" << i << "(fd=" << servers[i].getFd() << ")";
		if (i + 1 < servers.size()) oss << " | ";
	}

	if (!clients.empty()) oss << " || ";

	bool first = true;
	for (std::unordered_map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (!first) oss << " | ";
		first = false;

		const Client &cl = it->second;
		std::string state = "IDLE";
		switch (cl.state)
		{
			case ClientState::HEADERS_RECEIVED: state = "HEADERS"; break;
			case ClientState::BODY_RECEIVED:    state = "BODY";    break;
			case ClientState::COMPLETE:         state = "COMP";    break;
			case ClientState::ERROR:            state = "ERROR";   break;
			case ClientState::IDLE:             default:           state = "IDLE";    break;
		}

		oss << "C" << cl.fd << "->S" << cl.server_index << "[" << state << "]";
	}

	std::cout << oss.str() << std::endl;
}

// // ================
// void ServerLoop::startServer(ParseConfig parse)
// {
// 	std::vector<InitConfig> &servers = parse.getServers();

// 	serverSetup(servers);

// 	initPollfd(servers);

// 	while (true)
// 	{
// 		int ready = poll(fds.data(), fds.size(), 10);
// 		if (ready < 0)
// 			std::cout << "Error poll()!" << std::endl;

// 		for (size_t i = 0; i < fds.size(); i++)
// 		{
// 			if (fds[i].revents & POLLIN)
// 			{

// 				for (size_t k = 0; k < fds.size(); k++)
// 				{
// 					std::cout << std::endl;
// 					std::cout << "fd for index: " << k << fds[k].fd << std::endl;
// 					std::cout << "event for index: " << k << fds[k].events << std::endl;
// 					std::cout << "revent for index: " << k << fds[k].revents << std::endl;
// 					std::cout << std::endl;
// 				}


// 				bool isSocketFd = false;
// 				for (size_t s = 0; s < servers.size(); ++s)
// 				{
// 					if (fds[i].fd == servers[s].getFd())
// 					{
// 						isSocketFd = true;
// 						break;
// 					}
// 				}

// 				if (isSocketFd)
// 				{
// 					std::cout << "we accept the new client" << std::endl;
// 					struct sockaddr_in client_addr;
// 					socklen_t client_len = sizeof(client_addr);

// 					int client_fd = accept(fds[i].fd, (struct sockaddr*)&client_addr, &client_len);
// 					if (client_fd < 0)
// 						std::cout << "Error client_fd" << std::endl;

// 					struct pollfd new_pollfd;
// 					new_pollfd.fd = client_fd;
// 					new_pollfd.events = POLLIN;
// 					fds.push_back(new_pollfd);

// 					clients[client_fd] = Client(client_fd);

// 					std::cout << "new client connected: " << client_fd << std::endl;
// 				}
// 				else
// 				{
// 					std::cout << "this is a request for a client, here we read, parse and then send the response" << std::endl;

// 					int client_fd = fds[i].fd;

// 					bool done = clients[client_fd].request.receiveReq(client_fd);

// 					if (done)
// 					{
// 						std::cout << "entered done loop" << std::endl;
// 						if (!clients[client_fd].request.disconnect)
// 						{
// 							clients[client_fd].request.parseRequestFromCompleteBuffer();

// 							std::string responseStr = clients[client_fd].response.buildResponse(clients[client_fd].request);

// 							int bytes_sent = send(client_fd, responseStr.c_str(), responseStr.size(), 0);
// 							if (bytes_sent < 0)
// 							{
// 								perror("send");
// 								break;
// 							}

// 							std::cout << "response send to: " << fds[i].fd << std::endl;

// 							std::cout << "fd removed: " << fds[i].fd << std::endl;
// 							close(fds[i].fd);
// 							fds.erase(fds.begin() + i);
// 							i--;
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}
// }

// void ServerLoop::removeFd(std::vector<pollfd> &fds, size_t index)
// {
//     if (fds[index].fd >= 0)
//     {
//         std::cout << "Closing fd: " << fds[index].fd << std::endl;
//         close(fds[index].fd);
//     }
//     fds.erase(fds.begin() + index);
//     std::cout << "Removed fd at index: " << index << std::endl;
// }


// void ServerLoop::initPollfd(std::vector<InitConfig> &servers)
// {
// 	for (size_t i = 0; i < servers.size(); i++)
// 	{
// 		pollfd pfd;

// 		pfd.fd = servers[i].getFd();
// 		pfd.events = POLLIN;
// 		fds.push_back(pfd);

// 		if (fcntl(pfd.fd, F_SETFL, O_NONBLOCK) == -1)
// 			std::cout << "Error: setting fd to nonblock" << std::endl;
// 	}
// }

// void ServerLoop::serverSetup(std::vector<InitConfig> &servers)
// {
// 	std::cout << "servers being initialized..." << std::endl;     //logger

// 	for (size_t i = 0; i < servers.size(); i++)
// 	{
// 		bool duplicate = false;

// 		for (size_t j = 0; j < i; j++)
// 		{
// 			if (servers[j].getHost() == servers[i].getHost() &&
// 				servers[j].getPort() == servers[i].getPort())
// 			{
// 				throw ConfigError("Error: do not support virtual host!");
// 				duplicate = true;
// 				break;
// 			}
// 		}

// 		if (!duplicate)
// 		{
// 			if (!servers[i].createAndBindSocket())
// 				throw ConfigError("Failed to setup server socket!");
// 		}
// 		// std::cout << servers[0].getFd() << " after socket creation" << std::endl;
// 		// std::cout << "Server created 'host: ... ', port: '...'" << std::endl;     //logger
// 		std::cout << "Server socket created and bound successfully: "
//           << "Host: " << servers[i].getHost()
//           << ", Port: " << servers[i].getPort()
//           << ", Fd: " << servers[i].getFd()
//           << std::endl;
// 		std::cout << "Server created 'host: ... ', port: '...'" << std::endl;     //logger
// 	}
// }
