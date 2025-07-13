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

	// Print error pages
	std::cout << "Error Pages: ";
	if (error_pages.empty())
		std::cout << "none" << std::endl;
	else
	{
		std::cout << std::endl;
		for (std::map<short, std::string>::const_iterator it = error_pages.begin(); it != error_pages.end(); ++it)
			std::cout << "  " << it->first << " -> " << it->second << std::endl;
	}

	// Print locations (for later, after you parse them)
	std::cout << "Locations: " << locations.size() << std::endl;
}