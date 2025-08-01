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

};