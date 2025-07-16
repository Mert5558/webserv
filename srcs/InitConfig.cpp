#include "../inc/Webserv.hpp"
#include "../inc/InitConfig.hpp"

InitConfig::InitConfig()
{
	this->port = 0;
	this->host = "";
	this->server_name = "";
	this->root = "";
	this->index = "";
	this->autoindex = false;
	this->client_max_body_size = 1000000;
	this->listen_fd = -1;
	//errorpages
}

InitConfig::~InitConfig()
{}

InitConfig::InitConfig(const InitConfig &copy)
{
	port = copy.port;
	host = copy.host;
	server_name = copy.server_name;
	root = copy.root;
	index = copy.index;
	autoindex = copy.autoindex;
	client_max_body_size = copy.client_max_body_size;
	error_pages = copy.error_pages;
	locations = copy.locations;
	server_address = copy.server_address;
	listen_fd = copy.listen_fd;
}

InitConfig &InitConfig::operator=(const InitConfig &copy)
{
	if (this != &copy)
	{
		port = copy.port;
		host = copy.host;
		server_name = copy.server_name;
		root = copy.root;
		index = copy.index;
		autoindex = copy.autoindex;
		client_max_body_size = copy.client_max_body_size;
		error_pages = copy.error_pages;
		locations = copy.locations;
		server_address = copy.server_address;
		listen_fd = copy.listen_fd;
	}
	return (*this);
}

void InitConfig::parseLocation(std::vector<std::string> &location_lines, Location &loc)
{
	bool root_set = false, index_set = false, autoindex_set = false, client_max_body_size_set = false;
	bool allow_methods_set = false, return_set = false, alias_set = false, cgi_path_set = false, cgi_ext_set = false;

	std::set<short> error_codes_set;

	int depth = 0;
	for (size_t i = 0; i < location_lines.size(); i++)
	{
		const std::string &line = location_lines[i];

		if (i == 0)
		{
			std::istringstream iss(line);
			std::string key;
			std::string path;
			iss >> key >> path;
			loc.setPath(path);
			if (line.find("{") != std::string::npos)
				depth++;
			continue;
		}
		if (line == "{")
		{
			depth++;
			continue;
		}
		if (line == "}")
		{
			depth--;
			if (depth <= 0)
				break;
			continue;
		}
		if (line.empty())
			continue;
		
		std::istringstream iss(line);
		std::string key;
		std::string value;
		iss >> key;
		std::getline(iss, value);
		size_t value_start = value.find_first_not_of(" \t\r");

		if(value_start != std::string::npos)
			value = value.substr(value_start);
		else
			value = "";
		
		if (key == "root")
		{
			if (root_set)
				std::cout << "Error: 'root' duplivated in location!" << std::endl;
			loc.setRoot(value);
			root_set = true;
		}
		else if (key == "index")
		{
			if (index_set)
				std::cout << "Error: 'index' duplivated in location!" << std::endl;
			loc.setIndex(value);
			index_set = true;
		}
		else if (key == "autoindex")
		{
			if (autoindex_set)
				std::cout << "Error: 'autoindex' duplivated in location!" << std::endl;
			loc.setAutoindex(value);
			autoindex_set = true;
		}
		else if (key == "client_max_body_size")
		{
			if (client_max_body_size_set)
				std::cout << "Error: 'client_max_body_size' duplivated in location!" << std::endl;
			loc.setClientMaxBodySize(value);
			client_max_body_size_set = true;
		}
		else if (key == "allow_methods")
		{
			if (allow_methods_set)
				std::cout << "Error: 'allow_methods' duplivated in location!" << std::endl;
			std::istringstream methodss(value);
			std::vector<std::string> methods_vec;
			std::string method;
			while (methodss >> method)
				methods_vec.push_back(method);
			loc.setMethods(methods_vec);
			allow_methods_set = true;
		}
		else if (key == "return")
		{
			if (return_set)
				std::cout << "Error: 'return' duplivated in location!" << std::endl;
			loc.setReturn(value);
			return_set = true;
		}
		else if (key == "alias")
		{
			if (alias_set)
				std::cout << "Error: 'alias' duplivated in location!" << std::endl;
			loc.setAlias(value);
			alias_set = true;
		}
		else if (key == "cgi_path")
		{
			if (cgi_path_set)
				std::cout << "Error: 'cgi_path' duplivated in location!" << std::endl;
			loc.setCgiPath(value);
			cgi_path_set = true;
		}
		else if (key == "cgi_ext")
		{
			if (cgi_ext_set)
				std::cout << "Error: 'cgi_ext' duplivated in location!" << std::endl;
			loc.setCgiExt(value);
			cgi_ext_set = true;
		}
		else
			std::cout << "Error: Unknown setting " << key << " in location block" << std::endl;
	}
}

void InitConfig::setPort(std::string port)
{
	this->port = static_cast<uint16_t>(std::atoi(port.c_str()));
}

void InitConfig::setHost(std::string host)
{
	this->host = host;
}

void InitConfig::setServerName(std::string serverName)
{
	this->server_name = serverName;
}

void InitConfig::setRoot(std::string root)
{
	this->root = root;
}

void InitConfig::setIndex(std::string index)
{
	this->index = index;
}

void InitConfig::setAutoindex(std::string autoindex)
{
	if (autoindex == "on" || autoindex == "true" || autoindex == "1")
		this->autoindex = true;
	else
		this->autoindex = false;
}

void InitConfig::setClientMaxBodsize(std::string cmbs)
{
	this->client_max_body_size = std::strtoul(cmbs.c_str(), NULL, 10);
}

bool InitConfig::setErrorPage(std::string errorpage)
{
	std::istringstream iss(errorpage);
	short code;
	std::string path;
	iss >> code >> path;
	if (error_pages.count(code))
		return (false);
	if (!path.empty())
		this->error_pages[code] = path;
	return (true);
}

void InitConfig::addLocation(Location &loc)
{
	this->locations.push_back(loc);
}

void InitConfig::print() const
{
	std::cout << "---- Server Config ----" << std::endl;
	std::cout << "Port: " << port << std::endl;
	std::cout << "Host: " << host << std::endl;
	std::cout << "Server Name: " << server_name << std::endl;
	std::cout << "Root: " << root << std::endl;
	std::cout << "Index: " << index << std::endl;
	std::cout << "Autoindex: " << (autoindex ? "on" : "off") << std::endl;
	std::cout << "Client Max Body Size: " << client_max_body_size << std::endl;

	std::cout << "Error Pages:\n";
	if (error_pages.empty())
		std::cout << "  (none)\n";
	else
	{
		for (std::map<short, std::string>::const_iterator it = error_pages.begin(); it != error_pages.end(); ++it)
			std::cout << "  " << it->first << " -> " << it->second << std::endl;
	}

	std::cout << "Locations: " << locations.size() << std::endl;
	for (size_t i = 0; i < locations.size(); ++i)
	{
		std::cout << "  --- Location #" << (i+1) << " ---" << std::endl;
		locations[i].print(); // Assumes you have a print() function in Location
	}
}