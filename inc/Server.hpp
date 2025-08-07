#pragma once

#include "Webserv.hpp"
#include "ParseConfig.hpp"
#include "ParseHttp.hpp"
#include "Client.hpp"

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
		bool receiveReq(int client_fd, std::unordered_map<int, Client> &clients);

};