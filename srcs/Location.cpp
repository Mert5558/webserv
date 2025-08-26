#include "../inc/Webserv.hpp"
#include "../inc/Location.hpp"

Location::Location()
{
	this->path = "";
	this->root = "";
	this->index = "";
	this->autoindex = false;
	this->_return = "";
	this->alias = "";
	this->client_max_body_size = 1000000;
	this->methods.push_back(0); //GET
	this->methods.push_back(1); //POST
	this->methods.push_back(2); //DELETE
}

Location::Location(const Location &copy)
{
	path = copy.path;
	root = copy.root;
	index = copy.index;
	autoindex = copy.autoindex;
	methods = copy.methods;
	_return = copy._return;
	alias = copy.alias;
	cgi_path = copy.cgi_path;
	cgi_ext = copy.cgi_ext;
	client_max_body_size = copy.client_max_body_size;
}

Location &Location::operator=(const Location &copy)
{
	if (this != &copy)
	{
		path = copy.path;
		root = copy.root;
		index = copy.index;
		autoindex = copy.autoindex;
		methods = copy.methods;
		_return = copy._return;
		alias = copy.alias;
		cgi_path = copy.cgi_path;
		cgi_ext = copy.cgi_ext;
		client_max_body_size = copy.client_max_body_size;
	}
	return (*this);
}

Location::~Location()
{}

void Location::setPath(const std::string &path)
{
	this->path = path;
}

void Location::setRoot(const std::string &root)
{
	std::string cleanRoot = root;
	if (cleanRoot.compare(0, 2, "./") == 0)
		cleanRoot = cleanRoot.substr(2);
	this->root = cleanRoot;
}

void Location::setIndex(const std::string &index)
{
	this->index = index;
}

void Location::setAutoindex(const std::string &autoindex)
{
	if (autoindex == "on" || autoindex == "true" || autoindex == "1"
		|| autoindex == "on;" || autoindex == "true;" || autoindex == "1;")
		this->autoindex = true;
	else
		this->autoindex = false;
}

void Location::setClientMaxBodySize(const std::string &cmbs)
{
	this->client_max_body_size = std::strtoul(cmbs.c_str(), NULL, 10);
}

void Location::setMethods(const std::vector<std::string> &methods_vec)
{
	methods.clear();
	for (size_t i = 0; i < methods_vec.size(); i++)
	{
		std::string met = methods_vec[i];
		if (!met.empty() && met[met.size()-1] == ';')
			met = met.substr(0, met.size()-1);

		if (met == "GET")
			methods.push_back(0);
		else if (met == "POST")
			methods.push_back(1);
		else if (met == "DELETE")
			methods.push_back(2);
		else
			throw ConfigError("Method not allowed!");
	}
}

void Location::setReturn(const std::string &returnStr)
{
	this->_return = returnStr;
}

void Location::setAlias(const std::string &alias)
{
	this->alias = alias;
}

void Location::setCgiPath(const std::string &cgiPath)
{
	this->cgi_path.clear();
	std::istringstream iss(cgiPath);
	std::string path;
	while (iss >> path)
		this->cgi_path.push_back(path);
}

void Location::setCgiExt(const std::string &cgiExt)
{
	this->cgi_ext.clear();
	std::istringstream iss(cgiExt);
	std::string ext;
	while (iss >> ext)
		this->cgi_ext.push_back(ext);
}

const std::string &Location::getPath() const
{
	return (path);
}

const std::string &Location::getRoot() const
{
	return (root);
}

const std::string &Location::getIndex()
{
	return (index);
}

bool Location::getAutoindex()
{
	return (autoindex);
}

unsigned long Location::getClientMaxBodySize()
{
	return (client_max_body_size);
}

const std::vector<short> &Location::getMethods()
{
	return (methods);
}

const std::string &Location::getReturn()
{
	return (_return);
}

const std::string &Location::getAlias()
{
	return (alias);
}

const std::vector<std::string> &Location::getCgiPath() const
{
	return (cgi_path);
}

const std::vector<std::string> &Location::getCgiExt() const
{
	return (cgi_ext);
}

void Location::print() const
{
	std::cout << "    Path: " << path << std::endl;
	std::cout << "    Root: " << root << std::endl;
	std::cout << "    Index: " << index << std::endl;
	std::cout << "    Autoindex: " << (autoindex ? "on" : "off") << std::endl;
	std::cout << "    Client Max Body Size: " << client_max_body_size << std::endl;
	std::cout << "    Methods: ";
	for (size_t j = 0; j < methods.size(); ++j)
		std::cout << methods[j] << " ";
	std::cout << std::endl;
	std::cout << "    Return: " << _return << std::endl;
	std::cout << "    Alias: " << alias << std::endl;

	// Print CGI path(s) and ext(s) if you have them as vectors:
	std::cout << "    CGI Paths: ";
	for (size_t k = 0; k < cgi_path.size(); ++k)
		std::cout << cgi_path[k] << " ";
	std::cout << std::endl;

	std::cout << "    CGI Exts: ";
	for (size_t k = 0; k < cgi_ext.size(); ++k)
		std::cout << cgi_ext[k] << " ";
	std::cout << std::endl;
}


bool Location::isMethodAllowed(const std::vector<short> &methods, short method)
{
	return std::find(methods.begin(), methods.end(), method) != methods.end();
}
