/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseHttp.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkaratsi <kkaratsi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 21:33:30 by kkaratsi          #+#    #+#             */
/*   Updated: 2025/08/05 23:41:40 by kkaratsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Webserv.hpp"
#include "../inc/ParseHttp.hpp"
#include "../inc/ParseConfig.hpp"

// ParseConfig trimed;

HttpRequest::HttpRequest()
{
    this->method = Method::INVALID;
    this->path = "";
    this->version = Version::INVALID;
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

void HttpRequest::setMethod(Method method)
{
    this->method = method;
}

void HttpRequest::setPath(const std::string &path)
{
	this->path = path;
}

void HttpRequest::setVersion(Version version)
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
    switch(method)
    {
        case Method::GET:
            return "GET";
        case Method::POST:
            return "POST";
        case Method::DELETE:
            return "DELETE";
        default:
            return "INVALID";
    }
}

std::string HttpRequest::getPath() const
{
    return (path);
}

std::string HttpRequest::getVersion() const
{
    switch(version)
    {
        case Version::HTTP_1_0:
            return "HTTP/1.0";
        case Version::HTTP_1_1:
            return "HTTP/1.1";
        case Version::HTTP_2:
            return "HTTP/2";
        default:
            return "INVALID";
    }
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
    std::string methodStr;
    std::string versionStr;

     // Clear headers before parsing a new request
     headers.clear();
     
    // Parse the first line
    if (std::getline(raw, line))
    {
        std::istringstream requestLine(line);
        requestLine >> methodStr >> path >> versionStr;
    }
    
    method = toMethodEnum(methodStr);
    version = toVersionEnum(versionStr);
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

Method		HttpRequest::toMethodEnum(const std::string &methodStr)
{
    if (methodStr == "GET")
        return Method::GET;
    else if (methodStr == "POST")
        return Method::POST;
    else if (methodStr == "DELETE")
        return Method::DELETE;
    
    std::cerr << "Invalid HTTP method: " << methodStr << std::endl;
    return Method::INVALID;
}

Version     HttpRequest::toVersionEnum(const std::string &versionStr)
{
    if (versionStr == "HTTP/1.0")
        return Version::HTTP_1_0;
    else if (versionStr == "HTTP/1.1")
        return Version::HTTP_1_1;
    else if (versionStr == "HTTP/2")
        return Version::HTTP_2;

    std::cerr << "Invalid HTTP version: " << versionStr << std::endl;
    return Version::INVALID;
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
    std::cout << "\n" << getMethod() << " " << path << " " << getVersion() << std::endl;
}




bool    HttpRequest::isValidMethod() const
{
    return method != Method::INVALID;
}



bool HttpRequest::isValidVersion() const
{
    return version != Version::INVALID;
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
