#pragma once

#include "Webserv.hpp"


class Location
{
	private:
		std::string						path;
		std::string						root;
		std::string						index;
		bool							autoindex;
		std::vector<short>				methods;	// 0:GET, 1:POST, 2:DELETE
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
	
		void setPath(const std::string &path);
		void setRoot(const std::string &root);
		void setIndex(const std::string &index);
		void setAutoindex(const std::string &autoindex);
		void setClientMaxBodySize(const std::string &cmbs);
		void setMethods(const std::vector<std::string> &methods_vec);
		void setReturn(const std::string &retrunStr);
		void setAlias(const std::string &alias);
		void setCgiPath(const std::string &cgiPath);
		void setCgiExt(const std::string &cgiExt);

		const std::string &getPath() const;
		const std::string &getRoot() const;
		const std::string &getIndex();
		bool getAutoindex();
		unsigned long getClientMaxBodySize();
		const std::vector<short> &getMethods();
		const std::string &getReturn();
		const std::string &getAlias();

		const std::vector<std::string> &getCgiPath() const;
		const std::vector<std::string> &getCgiExt() const;
	
		bool isMethodAllowed(const std::vector<short> &methods, short method);
};