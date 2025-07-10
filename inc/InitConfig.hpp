#pragma once

#include "Webserv.hpp"
#include "Location.hpp"

class InitConfig
{
	private:
		uint16_t						port;
		std::string						host;
		std::string						server_name;
		std::string						root;
		std::string						index;
		bool							autoindex;
		unsigned long					client_max_body_size;
		std::map<short, std::string>	error_pages;
		std::vector<Location>			locations;
		struct sockaddr_in				server_address;
		int								listen_fd;

	public:
		InitConfig();
		InitConfig(const InitConfig &copy);
		InitConfig &operator=(const InitConfig &copy);
		~InitConfig();
};