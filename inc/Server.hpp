#pragma once

#include "Webserv.hpp"
#include "ParseConfig.hpp"
#include "HttpRequest.hpp"
#include "httpResponse.hpp"
#include "Client.hpp"

class Server
{
	private:
	
	public:
		Server();
		~Server();
		void startServer(ParseConfig parse);
		void serverSetup(std::vector<InitConfig> &servers);
		void parseHttp(std::vector<InitConfig> &servers, HttpRequest &request,  httpResponse &response);

		std::vector<pollfd> initPollfd(std::vector<InitConfig> &servers);


		void	removeFd(std::vector<pollfd> &fds, size_t index);
		bool	receiveReq(int client_fd, std::unordered_map<int, Client> &clients);

		bool	sendAll(int fd, const char* buffer, size_t length);
};