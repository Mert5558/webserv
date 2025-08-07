#pragma once

#include "Webserv.hpp"
#include "ParseConfig.hpp"
#include "ParseHttp.hpp"

class Server
{
	private:
	
	public:
		Server();
		~Server();
		void startServer(ParseConfig parse);
		void serverSetup(std::vector<InitConfig> &servers);
		void parseHttp(std::vector<InitConfig> &servers, HttpRequest &request);

		std::vector<pollfd> initPollfd(std::vector<InitConfig> &servers);

		void	removeFd(std::vector<pollfd> &fds, size_t index);
};