/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseHttp.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkaratsi <kkaratsi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 21:33:30 by kkaratsi          #+#    #+#             */
/*   Updated: 2025/07/30 14:18:06 by kkaratsi         ###   ########.fr       */
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

void HttpRequest::setHeaders(const std::vector<std::pair<std::string, std::string>> &headers)
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

std::vector<std::pair<std::string, std::string>> HttpRequest::getHeaders() const
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
    // std::cout << "----> Real Buffer: " << buffer << std::endl;
    return buffer;
}



bool    HttpRequest::parseRequest(const std::string &rawRequest)
{
    std::istringstream  raw(rawRequest);
    std::string line;
    std::string content_body;

     // Clear headers before parsing a new request
     headers.clear();
     
    // Parse the first line
    if (std::getline(raw, line))
    {
        std::istringstream requestLine(line);
        requestLine >> method >> path >> version;
    }

    log_first_line();

    // Parse headers
    while (std::getline(raw, line) && !line.empty() && line != "\r")
    {
        size_t delimiterPos = line.find(':');
        if (delimiterPos != std::string::npos)
        {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);
            headers.emplace_back(key, value);
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



void    HttpRequest::log_headers(const std::vector<std::pair<std::string, std::string>> &headers)
{
    
    for (auto it = headers.begin(); it != headers.end(); ++it)
    {
        std::cout << it->first << ":" << it->second << "\n";
    }
    
}

void    HttpRequest::log_first_line()
{
    std::cout << "\n" << method << " " << path << " " << version << std::endl;
}




bool    HttpRequest::isValidMethod()
{
    if (method == "GET")
    {
        // do something
        std::cout << "------- We have a correct GET method -------" << std::endl;
    }
    else if (method == "POST")
    {
        // do something else
    }
    else if (method == "DELETE")
    {
        // do something else
    }
    else
    {
        std::cout << "------- Invalid HTTP method -------" << std::endl;
        return false;
    }
    return true;
}



bool HttpRequest::isValidVersion()
{
    if (version == "HTTP/1.0" || version == "HTTP/1.1" || version == "HTTP/2")
    {
        std::cout << "------- Valid HTTP version: " << version << " -------" << std::endl;
        return true;
    }
    else
    {
        std::cout << "------- Invalid HTTP version: " << version << " -------" << std::endl;
        return false;
    }
}

bool HttpRequest::isValidPath()
{
    // Check if the path is empty
    if (path.empty())
    {
        std::cout << "------- Invalid Path: Path is empty -------" << std::endl;
        return false;
    }

    // Check if the path starts with a '/'
    if (path[0] != '/')
    {
        std::cout << "------- Invalid Path: Path does not start with '/' -------" << std::endl;
        return false;
    }

    // Check for invalid characters (e.g., spaces)
    if (path.find(' ') != std::string::npos)
    {
        std::cout << "------- Invalid Path: Path contains spaces -------" << std::endl;
        return false;
    }

    // Additional checks can be added here (e.g., length, specific patterns, etc.)
    std::cout << "------- Valid Path: " << path << " -------" << std::endl;
    return true;
}

// Function to read the contents of a file into a string
std::string HttpRequest::readFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return "./www/error/404.html";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}



std::string HttpRequest::buildResponse()
{
    std::string status;
    std::string content_type;
    std::string body;

    if(path == "/")
    {
        status = "HTTP/1.1 200 OK";
        content_type = "text/html; charset=utf-8";
        body = readFile("./www/index2.html");
    }
    else if (path == "/assets/images")
    {
        status = "HTTP/1.1 200 OK";
        content_type = "image/svg+xml";
        body = readFile("./www/assets/images/logo.svg");
    }
    else
    {
        status = "HTTP/1.1 404 Not Found";
        content_type = "text/html; charset=utf-8";
        body = readFile("./www/Error/404.html");
    }

    //Assemble the complete HTTP response
    std::ostringstream response;
    response << status << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "\r\n";
    response << body;

    // std::cout << "---> HTTP Response:\n" << response.str() << std::endl; // print the complete HTTP response
    return response.str();
}
