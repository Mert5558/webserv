#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <arpa/inet.h>


class ConfigError: public std::exception
{
	private:
		std::string _msg;
	public:
		ConfigError(const std::string &msg) throw()
			:_msg(msg) {}
		
		virtual ~ConfigError() throw () {};

		virtual const char *what() const throw()
		{
			return _msg.c_str();
		}
};