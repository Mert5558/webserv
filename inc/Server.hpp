#pragma once

#include "Webserv.hpp"
#include "ParseConfig.hpp"

class Server
{
	private:
	
	public:
		Server();
		~Server();
		void startServer(ParseConfig parse);
		void serverSetup(std::vector<InitConfig> servers);

};