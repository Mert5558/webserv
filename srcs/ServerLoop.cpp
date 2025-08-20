#include "../inc/ServerLoop.hpp"
#include "../inc/Webserv.hpp"
#include "../inc/ParseConfig.hpp"
#include "../inc/HttpRequest.hpp"
#include "../inc/HttpResponse.hpp"
#include "../inc/Client.hpp"
#include "../inc/InitConfig.hpp"
#include "../inc/Cgi.hpp"

#include <cerrno>     // for errno
#include <signal.h>   // for signal(SIGPIPE, SIG_IGN)

// Set a file descriptor to non-blocking mode.
static bool setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return false;
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		return false;
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
ServerLoop::ServerLoop() {}
ServerLoop::~ServerLoop() {}

// Setup each server socket: create, bind, listen, and set non-blocking.
void ServerLoop::serverSetup(std::vector<InitConfig> &servers)
{
	std::cout << "servers being initialized..." << std::endl;

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
// ! This is now the ONLY place that closes fds.
void ServerLoop::removeFd(std::vector<pollfd> &fds, size_t index)
{
	if (index >= fds.size())
		return;

	if (fds[index].fd >= 0)
	{
		std::cout << "Closing fd: " << fds[index].fd << std::endl;
		close(fds[index].fd);
	}

	fds.erase(fds.begin() + index);
	std::cout << "(-) Removed fd at index: " << index << std::endl;
}

// Build HTTP response based on request and server config
// void ServerLoop::parseHttp(std::vector<InitConfig> &servers, HttpRequest &request, HttpResponse &response)
// {
// 	InitConfig *srv = servers.empty() ? NULL : &servers[0];
// 	response.prepare(request, srv);
// }



static const Location* findLocation(const InitConfig &srv, const std::string &reqPath)
{
	const std::vector<Location> &locs = srv.getLocations(); // assumes this exists
    const Location *best = NULL;
    size_t bestLen = 0;
    for (size_t i = 0; i < locs.size(); ++i)
	{
        const std::string &lp = locs[i].getPath();
        if (!lp.empty() && reqPath.rfind(lp, 0) == 0) { // prefix
            if (lp.size() > bestLen)
			{
                best = &locs[i];
                bestLen = lp.size();
            }
        }
    }
    return best;
}


void ServerLoop::parseHttp(std::vector<InitConfig> &servers, HttpRequest &request, HttpResponse &response)
{
    InitConfig *srv = servers.empty() ? NULL : &servers[0];
    if (!srv) {
        response.prepare(request, NULL);
        return;
    }

    const std::string reqPath = request.getPath();
    const Location *loc = findLocation(*srv, reqPath);

    bool isCgi = false;
    const Location *cgiLoc = NULL;
    std::string scriptFsPath;

    if (loc) {
        // Check extension against configured CGI extensions
        const std::vector<std::string> &exts = loc->getCgiExt();
        if (!exts.empty())
		{
            size_t dot = reqPath.rfind('.');
            if (dot != std::string::npos)
			{
                std::string ext = reqPath.substr(dot);
                for (size_t i = 0; i < exts.size(); ++i)
				{
                    if (exts[i] == ext)
					{
                        isCgi = true;
                        cgiLoc = loc;
                        break;
                    }
                }
            }
        }
    }

    if (isCgi && cgiLoc)
	{
        try {
            // Build filesystem path: root + remainder after location prefix
            std::string locPrefix = cgiLoc->getPath();          // e.g. "/cgi-bin"
            std::string rel = reqPath.substr(locPrefix.size()); // may be "/script.py"
            if (!rel.empty() && rel[0] == '/')
                rel.erase(0,1);
            scriptFsPath = cgiLoc->getRoot();
            if (!scriptFsPath.empty() && scriptFsPath.back() != '/')
                scriptFsPath += '/';
            scriptFsPath += rel;

            // Prepare environment
            Location locCopy = *cgiLoc; // buildEnv wants non-const ref
            Client dummyClient;         // if you later store real client info, pass it
            std::map<std::string,std::string> env = Cgi::buildEnv(request, locCopy, dummyClient, scriptFsPath);

            Cgi cgi(scriptFsPath, env);
            auto res = cgi.execute("", locCopy);
            if (res.first == CgiStatus::SUCCESS)
			{
                // If CGI output already contains headers (e.g. "Content-Type: ...\r\n\r\n")
                // you could parse them. For now wrap as plain text.
                // response.clear(); // ensure a clean slate (implement if needed)
                response.setStatusCode("OK");        // adapt to your HttpResponse API
                response.addHeader("Content-Type", "text/plain");
                response.setBody(res.second);
                return;
            }
			else
			{
				std::cerr << "ERROR 500" << std::endl;
                // response.prepareError(500); // adapt to your API
                return;
            }
        } catch (const std::exception &e)
		{
			std::cerr << "ERROR 500" << std::endl;
            // response.prepareError(500); // log e.what() if desired
            return;
        }
    }

    // Fallback: normal static handling
    response.prepare(request, srv);
}




// Debug: dump current server and client state
void ServerLoop::dumpTopology(const std::vector<InitConfig> &servers)
{
	std::ostringstream oss;
	oss << "[TOPO] ";

	for (size_t i = 0; i < servers.size(); ++i)
	{
		oss << "S" << i << "(fd=" << servers[i].getFd() << ")";
		if (i + 1 < servers.size())
			oss << " | ";
	}

	if (!clients.empty())
		oss << " || ";

	bool first = true;
	for (const auto &it : clients)
	{
		if (!first)
			oss << " | ";
		first = false;

		const Client &cl = it.second;
		std::string state = "IDLE";
		switch (cl.state)
		{
		case ClientState::HEADERS_RECEIVED:
			state = "HEADERS";
			break;
		case ClientState::BODY_RECEIVED:
			state = "BODY";
			break;
		case ClientState::COMPLETE:
			state = "COMP";
			break;
		case ClientState::ERROR:
			state = "ERROR";
			break;
		default:
			state = "IDLE";
			break;
		}

		oss << "C" << cl.fd << "->S" << cl.server_index << "[" << state << "]";
	}

	std::cout << oss.str() << std::endl;
}

void ServerLoop::startServer(ParseConfig parse)
{
	// Prevent process-kill on write to a closed socket
	signal(SIGPIPE, SIG_IGN);

	std::vector<InitConfig> &servers = parse.getServers();
	if (servers.empty())
		throw ConfigError("No servers configured!");

	std::cout << "[BOOT] serverSetup()..." << std::endl;
	serverSetup(servers);
	std::cout << "[BOOT] initPollfd()..." << std::endl;
	initPollfd(servers);
	std::cout << "[READY] Poll vector size: " << fds.size() << " (listening sockets added)" << std::endl;

	// Track last activity for timeout
	std::unordered_map<int, time_t> clientTimeouts;

	while (true)
	{
		int ret = poll(&fds[0], fds.size(), 100); // 100ms
		if (ret < 0)
		{
			if (errno == EINTR) continue;
			perror("poll");
			break;
		}

		time_t now = time(NULL);

		// === Timeout inactive clients ===
		for (auto it = clientTimeouts.begin(); it != clientTimeouts.end();)
		{
			if (now - it->second > 5)
			{
				int tfd = it->first;
				std::cout << "[TIMEOUT] fd=" << tfd << " closed due to inactivity" << std::endl;

				clients.erase(tfd);
				// Remove from fds (also closes fd)
				for (size_t i = 0; i < fds.size(); ++i)
				{
					if (fds[i].fd == tfd)
					{
						removeFd(fds, i); // single close point
						break;
					}
				}
				it = clientTimeouts.erase(it);
			}
			else
			{
				++it;
			}
		}

		// === Poll event loop ===
		size_t i = 0;
		while (i < fds.size())
		{
			pollfd &pfd = fds[i];
			int fd = pfd.fd;

			// Handle errors: POLLERR, POLLHUP, POLLNVAL
			if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				std::cout << "[EVT] fd=" << fd << " error/hup/nval (revents=" << pfd.revents << "), closing." << std::endl;
				if (clients.count(fd))
				{
					clients.erase(fd);
					clientTimeouts.erase(fd);
				}
				removeFd(fds, i); // closes fd
				continue; // do not increment i
			}

			// Is it a listening socket?
			int serverIdx = findServerIndexByFd(servers, fd);
			if (serverIdx >= 0)
			{
				if (pfd.revents & POLLIN)
				{
					while (true)
					{
						int cfd = accept(fd, NULL, NULL);
						if (cfd < 0)
						{
							if (errno == EAGAIN || errno == EWOULDBLOCK)
								break;
							perror("accept");
							break;
						}

						if (!setNonBlocking(cfd))
						{
							close(cfd);
							continue;
						}

						fds.push_back(pollfd{cfd, POLLIN, 0});
						clients[cfd] = Client(cfd, serverIdx);
						clientTimeouts[cfd] = now;

						std::cout << "[ACCEPT] New client: fd=" << cfd << " from server " << serverIdx << std::endl;
					}
				}
				++i;
				continue;
			}

			// It's a client socket → update activity
			clientTimeouts[fd] = now;

			// === Handle incoming data (POLLIN) ===
			if (pfd.revents & POLLIN)
			{
				Client &cl = clients[fd];

				// FIX: Treat EAGAIN/EWOULDBLOCK as non-fatal (keep the connection)
				bool ok = cl.request.receiveReq(fd);
				if (!ok)
				{
					if (errno == EAGAIN || errno == EWOULDBLOCK)
					{
						// Nothing to read right now—try again later
						++i;
						continue;
					}
				}

				if (!ok || cl.request.disconnect)
				{
					std::cout << "[READ] EOF or hard error on fd=" << fd << ", closing." << std::endl;
					clients.erase(fd);
					clientTimeouts.erase(fd);
					removeFd(fds, i); // closes fd
					continue;
				}

				ParseResult parseRes = cl.request.parse();
				if (parseRes == ParseResult::COMPLETE)
				{
					std::cout << "[READ] Request complete on fd=" << fd << std::endl;
					parseHttp(servers, cl.request, cl.response);
					cl.outBuf = cl.response.buildResponse();
					cl.outOff = 0;
					pfd.events = POLLOUT;
				}
				else if (parseRes == ParseResult::ERROR)
				{
					std::cout << "[PARSE] Error on fd=" << fd << ", closing." << std::endl;
					clients.erase(fd);
					clientTimeouts.erase(fd);
					removeFd(fds, i); // closes fd
					continue;
				}
			}

			// === Handle outgoing data (POLLOUT) ===
			if (pfd.revents & POLLOUT)
			{
				Client &cl = clients[fd];
				size_t left = cl.outBuf.size() - cl.outOff;
				const char *data = cl.outBuf.data() + cl.outOff;
				ssize_t n = send(fd, data, left, 0);

				if (n < 0)
				{
					if (errno == EAGAIN || errno == EWOULDBLOCK)
					{
						++i;
						continue;
					}
					std::cout << "[WRITE] Hard error on fd=" << fd << ", closing." << std::endl;
					clients.erase(fd);
					clientTimeouts.erase(fd);
					removeFd(fds, i); // closes fd
					continue;
				}

				cl.outOff += n;
				if (cl.outOff >= cl.outBuf.size())
				{
					std::cout << "[WRITE] Full response sent on fd=" << fd << ", closing." << std::endl;
					clients.erase(fd);
					clientTimeouts.erase(fd);
					removeFd(fds, i); // closes fd
					continue;
				}
			}

			++i;
		}

		// Debug: current topology
		// dumpTopology(servers);
	}
}
