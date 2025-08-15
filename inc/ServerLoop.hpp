#pragma once

#include "Webserv.hpp"
#include "ParseConfig.hpp"
#include "ParseHttp.hpp"
#include "httpResponse.hpp"
#include "Client.hpp"

class ServerLoop
{
	public:
		ServerLoop();
		~ServerLoop();

		// Boot/start the server loop
		void startServer(ParseConfig parse);
		void serverSetup(std::vector<InitConfig> &servers);
		void initPollfd(std::vector<InitConfig> &servers);

		// Parse HTTP requests and send responses - Must be moved
		void parseHttp(std::vector<InitConfig> &servers, HttpRequest &request, httpResponse &response); //to be moved to parseHttp.hpp

		// Remove a file descriptor from the pollfd vector
		void removeFd(std::vector<pollfd> &fds, size_t index);

		// Helper for debugging
		void dumpTopology(const std::vector<InitConfig> &servers);

		// State
		std::vector<pollfd>              fds;
		std::unordered_map<int, Client>  clients;
};

