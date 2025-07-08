#pragma once

#include "Webserv.hpp"

class Config
{
	int port;
	std::string root;
	std::string index;

	Config();
	bool parse(const std::string &filepath);
};