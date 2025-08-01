#pragma once

#include "Webserv.hpp"
#include "InitConfig.hpp"

class ParseConfig
{
	private:
		std::vector<InitConfig> servers;
		std::vector<std::string> blocks;
		int blockNB;

	public:
	ParseConfig();
	~ParseConfig();
	
	int parseFile(std::string configfile);

	bool isFileReadable(const std::string &filename);
	bool isRegularFile(const std::string &filename);
	bool isFileEmpty(const std::string &filename);

	void removeComments(std::string &content);
	void trimWhitespaces(std::string &content);
	void extractServerBlocks(const std::string &content);
	bool isServerBlockEmpty(const std::string &block) const;
	std::vector<std::string> splitIntoLines(const std::string &content);
	void parseServerSettings(const std::vector<std::string> &lines, InitConfig &config);
	void validatePaths();
	void checkDupServers();
	std::vector<InitConfig> &getServers();

};