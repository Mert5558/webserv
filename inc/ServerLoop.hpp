#pragma once

#include "Webserv.hpp"
#include "ParseConfig.hpp"
#include "ParseHttp.hpp"
#include "httpResponse.hpp"
#include "Client.hpp"

class ServerLoop
{
	private:
	
	public:
		ServerLoop();
		~ServerLoop();
		void startServer(ParseConfig parse);
		void serverSetup(std::vector<InitConfig> &servers);

		void initPollfd(std::vector<InitConfig> &servers);

		bool	isSocketFd(int client_fd, std::vector<InitConfig> &servers);
		void	acceptClient(int client_fd);
		void	removeClient(int client_fd);
		void	updateFds();


		std::vector<pollfd> fds;
		std::unordered_map<int, Client> clients;

		std::vector<pollfd> toAdd;
		std::vector<int> toRemove;

};