#pragma once

#include "Webserv.hpp"

class Location
{
	private:
		std::string					path;
		std::string					root;
		std::string					index;
		bool						autoindex;
		std::vector<short>			methods;	// 0:GET, 1:POST, 3:DELETE
		std::string					_return;
		std::string					alias;
		std::vector<std::string>	cgi_path;
		std::vector<std::string>	cgi_ext;
		unsigned long				client_max_body_size;
	
	public:
		Location();
		Location(const Location &copy);
		Location &operator=(const Location &copy);
		~Location();
	
	
};