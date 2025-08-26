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
		
		
		std::pair<CgiStatus, std::string>			execute(const std::string &inputData, Location &loc);
		static std::map<std::string, std::string> 	buildEnv(const HttpRequest &req, Location	&loc, const Client	&client, InitConfig &server, const std::string	&scriptPath);
		std::string									findExtension(const std::string &scriptPath,const std::vector<std::string>& cgi_ext,const std::vector<std::string>& cgi_path);
	};