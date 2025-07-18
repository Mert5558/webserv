#pragma once

#include "Webserv.hpp"

class Location
{
	private:
		std::string						path;
		std::string						root;
		std::string						index;
		bool							autoindex;
		std::vector<short>				methods;	// 0:GET, 1:POST, 3:DELETE
		std::string						_return;
		std::string						alias;
		std::vector<std::string>		cgi_path;
		std::vector<std::string>		cgi_ext;
		unsigned long					client_max_body_size;
	
	public:
		Location();
		Location(const Location &copy);
		Location &operator=(const Location &copy);
		~Location();
	
		void setPath(std::string path);
		void setRoot(std::string root);
		void setIndex(std::string index);
		void setAutoindex(std::string autoindex);
		void setClientMaxBodySize(std::string cmbs);
		void setMethods(std::vector<std::string> methods_vec);
		void setReturn(std::string retrunStr);
		void setAlias(std::string alias);
		void setCgiPath(std::string cgiPath);
		void setCgiExt(std::string cgiExt);
		void print() const;

		std::string getPath();
		std::string getRoot();
		std::string getIndex();
		bool getAutoindex();
		unsigned long getClientMaxBodySize();
		std::vector<short> getMethods();
		std::string getReturn();
		std::string getAlias();
		std::vector<std::string> getCgiPath();
		std::vector<std::string> getCgiExt();
	
};