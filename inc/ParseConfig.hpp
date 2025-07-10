#pragma once

#include "Webserv.hpp"
#include "InitConfig.hpp"

class ParseConfig
{
	private:
		std::vector<InitConfig> servers;

	public:
	ParseConfig();
	~ParseConfig();
	
	int parseFile(std::string configfile);

	bool isFileReadable(const std::string &filename);
	bool isRegularFile(const std::string &filename);
	bool isFileEmpty(const std::string &filename);

	void removeComments(std::string &content);
	void trimWhitespaces(std::string &content);

};