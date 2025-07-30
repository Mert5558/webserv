#include "../inc/Server.hpp"
#include "../inc/ParseConfig.hpp"

Server::Server()
{}

Server::~Server()
{}

void Server::startServer(ParseConfig parse)
{
	//stuff

	serverSetup(parse.getServers());
	// while (true)
	// 	sleep(1);
}

void Server::serverSetup(std::vector<InitConfig> servers)
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
			std::cout << "-----------------------------------------------" << std::endl;
			if (!servers[i].createAndBindSocket())
				throw ConfigError("Failed to setup server socket!");
		}
		std::cout << "Server created 'host: ... ', port: '...'" << std::endl;     //logger
	}
}
