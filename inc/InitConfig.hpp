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

		uint16_t getPort();
		std::string getHost();
		std::string getServerName();
		std::string getRoot();
		std::string getIndex();
		bool getAutoindex();
		unsigned long getClientMaxBodySize();
		std::map<short, std::string> getErrorPages();
		std::vector<Location> getLocations();


		void setPort(std::string port);
		void setHost(std::string host);
		void setServerName(std::string serverName);
		void setRoot(std::string root);
		void setIndex(std::string index);
		void setAutoindex(std::string autoindex);
		void setClientMaxBodsize(std::string cmbs);
		bool setErrorPage(std::string errorpage);
		void print() const;
		void parseLocation(std::vector<std::string> &location_lines, Location &loc);
		void addLocation(Location &loc);

};