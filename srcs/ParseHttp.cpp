/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseHttp.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkaratsi <kkaratsi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 21:33:30 by kkaratsi          #+#    #+#             */
/*   Updated: 2025/07/28 22:51:08 by kkaratsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Webserv.hpp"
#include "../inc/ParseHttp.hpp"

HttpRequest::HttpRequest()
{
    this->method = "";
    this->path = "";
    this->version = "";
    this->body = "";
    this->headers = {};
}

HttpRequest::~HttpRequest()
{}

HttpRequest::HttpRequest(const HttpRequest &copy)
{
    method = copy.method;
    path = copy.path;
    version = copy.version;
    body = copy.body;
    headers = copy.headers;
}

HttpRequest &HttpRequest::operator=(const HttpRequest &copy)
{
    if (this != &copy)
    {
        method = copy.method;
        path = copy.path;
        version = copy.version;
        body = copy.body;
        headers = copy.headers;
    }
    return (*this);
}



/* Setter */
void HttpRequest::setMethod(const std::string &method)
{
	this->method = method;
}

void HttpRequest::setPath(const std::string &path)
{
	this->path = path;
}

void HttpRequest::setVersion(const std::string &version)
{
	this->version = version;
}

void HttpRequest::setBody(const std::string &body)
{
	this->body = body;
}

void HttpRequest::setHeaders(const std::unordered_map<std::string, std::string> &headers)
{
    this->headers = headers;
}




/* Getter */
std::string HttpRequest::getMethod() const
{
    return (method);
}

std::string HttpRequest::getPath() const
{
    return (path);
}

std::string HttpRequest::getVersion() const
{
    return (version);
}

std::string HttpRequest::getBody() const
{
    return (body);
}

std::unordered_map<std::string, std::string> HttpRequest::getHeaders() const
{
    return (headers);
}
