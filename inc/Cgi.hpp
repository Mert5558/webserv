#pragma once

#include "Webserv.hpp"
#include "HttpRequest.hpp"
#include "Location.hpp"
#include "Client.hpp"

enum class CgiStatus {SUCCESS, EXECUTION_ERROR};

class Cgi
{
	private:
		std::string									scriptPath;
		std::map<std::string, std::string>			env;
		std::vector<char *>							makeEnv() const;
		
		public:
		Cgi();
		~Cgi();
		Cgi(const std::string &scriptPath, const std::map<std::string, std::string> &env);
		
		
		std::pair<CgiStatus, std::string>			execute(const std::string &inputData);
		static std::map<std::string, std::string> 	buildEnv(const HttpRequest &req, Location	&loc, const Client	&client, const std::string	&scriptPath);
	};