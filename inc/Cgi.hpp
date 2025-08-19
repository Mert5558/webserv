#pragma once

#include "Webserv.hpp"

enum class CgiStatus {SUCCESS, EXECUTION_ERROR};

class Cgi
{
	private:
		std::string									scriptPath;
		const std::map<std::string, std::string>	env;
		std::vector<char *>							setupEnv();
		
		public:
		Cgi();
		~Cgi();
		Cgi(const std::string &scriptPath, const std::map<std::string, std::string> &env);
		
		
		std::pair<CgiStatus, std::string>			execute(const std::string &inputData);
	};