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

		void startServer(ParseConfig parse);
		void serverSetup(std::vector<InitConfig> &servers);
		void parseHttp(std::vector<InitConfig> &servers, HttpRequest &request, httpResponse &response); //to be moved to parseHttp.hpp

		void initPollfd(std::vector<InitConfig> &servers);

		void removeFd(std::vector<pollfd> &fds, size_t index);

		std::vector<pollfd>              fds;
		std::unordered_map<int, Client>  clients;
};

