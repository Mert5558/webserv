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
		struct sockaddr_in				socket_address;
		int								listen_fd;

	public:
		InitConfig();
		InitConfig(const InitConfig &copy);
		InitConfig &operator=(const InitConfig &copy);
		~InitConfig();

		uint16_t getPort() const;
		const std::string &getHost() const;
		const std::string &getServerName();
		const std::string &getRoot() const;
		const std::string &getIndex() const;
		int getFd() const;
		bool getAutoIndex() const;
		unsigned long getClientMaxBodySize();
		const std::map<short, std::string> &getErrorPages();
		const std::vector<Location> &getLocations();


		void setPort(const std::string &port);
		void setHost(const std::string &host);
		void setServerName(const std::string &serverName);
		void setRoot(const std::string &root);
		void setIndex(const std::string &index);
		void setAutoindex(const std::string &autoindex);
		void setClientMaxBodsize(const std::string &cmbs);
		bool setErrorPage(const std::string &errorpage);
		void setFd(int fd);


		void print() const;
		void parseLocation(std::vector<std::string> &location_lines, Location &loc);
		void addLocation(Location &loc);
		bool createAndBindSocket();

};