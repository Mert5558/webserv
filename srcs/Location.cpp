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