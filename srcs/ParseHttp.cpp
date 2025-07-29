/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseHttp.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkaratsi <kkaratsi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 21:33:30 by kkaratsi          #+#    #+#             */
/*   Updated: 2025/07/29 17:15:02 by kkaratsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Webserv.hpp"
#include "../inc/ParseHttp.hpp"
#include "../inc/ParseConfig.hpp"

// ParseConfig trimed;

HttpRequest::HttpRequest()
{
    this->method = "";
    this->path = "";
    this->version = "";
    this->body = "";
    this->headers = {};
}

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

HttpRequest::~HttpRequest()
{}



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



std::string HttpRequest::receiveRequest(int client_fd)
{
    std::string buffer;
    char chunk[1024] = {0};
    unsigned long bytes_read;

    while ((bytes_read = recv(client_fd, chunk, sizeof(chunk) - 1, 0)) > 0)
    {
        chunk[bytes_read] = '\0'; // Null-terminate the chunk
        buffer.append(chunk);    // Append to the buffer
        if (bytes_read < sizeof(chunk) - 1) break; // End of request
        if (bytes_read <= 0)																		
        {
            std::cout << "Client disconnected or error occurred." << std::endl;
            break;
        }
    }
    std::cout << "~~~~~~~~~~~~~~~Real Buffer: " << buffer << std::endl;
    return buffer;
}

bool    HttpRequest::parseRequest(const std::string &rawRequest)
{
    std::istringstream  raw(rawRequest);
    std::string line;
    std::string content_body;

    // Parse the first line
    if (std::getline(raw, line))
    {
        std::istringstream requestLine(line);
        requestLine >> method >> path >> version;
    }
    log_first_line();

    // Parse headers
    while (std::getline(raw, line) && !line.empty())
    {
        size_t delimiterPos = line.find(':');
        if (delimiterPos != std::string::npos)
        {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);
            headers[key] = value;
        }
    }
    
    log_headers(headers);

    //Parse body
    while(std::getline(raw, line))
    {
        content_body += line + "\n";
    }
    body = content_body;
    
    
    return true;
}


void    HttpRequest::log_headers(const std::unordered_map<std::string, std::string> &headers)
{
    // Copy headers into a vector
    std::vector<std::pair<std::string, std::string>> headerVector(headers.begin(), headers.end());
    
    // Reverse iterate over the vector
    for (auto it = headerVector.rbegin(); it != headerVector.rend(); ++it)
    {
        std::cout << it->first << ":" << it->second << "\n";
    }
}

// void    HttpRequest::log_headers(const std::unordered_map<std::string, std::string> &headers)
// {
    
//     // Reverse iterate over the vector
//     for (auto it = headers.begin(); it != headers.end(); ++it)
//     {
//         std::cout << it->first << ":" << it->second << "\n";
//     }
// }

void    HttpRequest::log_first_line()
{
    std::cout << method << " " << path << " " << version << std::endl;
}
