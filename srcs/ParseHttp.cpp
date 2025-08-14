/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseHttp.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 21:33:30 by kkaratsi          #+#    #+#             */
/*   Updated: 2025/08/12 15:41:35 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Webserv.hpp"
#include "../inc/ParseHttp.hpp"
#include "../inc/ParseConfig.hpp"

// ParseConfig trimed;

HttpRequest::HttpRequest() : bodySize(0), parseState(ParseState::START_LINE)
{
    this->method = Method::INVALID;
    this->path = "";
    this->version = Version::INVALID;
    this->headers = {};
}

HttpRequest::HttpRequest(const HttpRequest &copy)
{
    method = copy.method;
    path = copy.path;
    version = copy.version;
    headers = copy.headers;
}

HttpRequest &HttpRequest::operator=(const HttpRequest &copy)
{
    if (this != &copy)
    {
        method = copy.method;
        path = copy.path;
        version = copy.version;
        headers = copy.headers;
    }
    return (*this);
}

HttpRequest::~HttpRequest()
{
    if (bodyFile.is_open())
    {
        bodyFile.close();
    }
}



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

void HttpRequest::setBody(const std::string &filePath)
{
	if(this->bodyFile.is_open())
        this->bodyFile.close();

    this->bodyFile.open(filePath, std::ios::out | std::ios::trunc);
    if (!bodyFile.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filePath);
    }
    this->bodyFilePath = filePath;

    std::cout << "Body file set to: " << filePath << std::endl;
}

void HttpRequest::setHeaders(const std::unordered_map<std::string, std::string> &headers)
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

std::string HttpRequest::getBodyFilePath() const
{
    return this->bodyFilePath; 
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
    // std::cout << "----> Real Buffer: " << buffer << std::endl;
    return buffer;
}


ssize_t HttpRequest::receive(int client_fd, std::string &buffer)
{
    char tmp[2048];
    ssize_t bytes = 0;

    bytes = recv(client_fd, tmp, sizeof(tmp), 0);
    if (bytes > 0)
    {
        buffer.append(tmp, bytes);
    }
    return bytes;
}


void    HttpRequest::reset()
{
    //std::cout << "\nCalled the reset() ---> method = Method::INVALID | path.clear() | version = Version::INVALID | headers.clear() | parseState = ParseState::START_LINE  " << std::endl;
    
    method = Method::INVALID;
    path.clear();
    version = Version::INVALID;
    headers.clear();
    if (bodyFile.is_open())
    {
        bodyFile.close();
    }
    parseState = ParseState::START_LINE;

	rawRequest.clear();
	header_str.clear();
}



bool    HttpRequest::parseStartLine(const std::string &line)
{
    std::istringstream requestLine(line);
    std::string methodStr, versionStr;
    
    requestLine >> methodStr >> path >> versionStr;

    method = toMethodEnum(methodStr);
    version = toVersionEnum(versionStr);
    
    if ( method == Method::INVALID || version == Version::INVALID)
    {
        // std::cerr << "Invalid start line " << line << std::endl;
        return false;
    }
    
    return true;
}

bool    HttpRequest::parseHeaders(const std::string &line)
{
    size_t delimiterPos = line.find(':');
    
    if (delimiterPos == std::string::npos)
    {
        std::cerr << "Invalid header: " << line << std::endl;
        return false;
    }

    std::string key = line.substr(0, delimiterPos);
    std::string value = line.substr(delimiterPos + 1);

    // Trim whitespace around key and value
    key.erase(key.find_last_not_of(" \t\r\n") + 1);
    value.erase(0, value.find_first_not_of(" \t\r\n"));

    headers[key] = value;
    
    return true;   
}




ParseResult HttpRequest::parseRequestPartial(std::string &buffer)
{
    while (true)
    {
        // debug state of parsing
        // std::cout << "Current parse state: " << static_cast<int>(parseState) << std::endl;
        
        switch (parseState)
        {
            case ParseState::START_LINE:
            {
                size_t pos = buffer.find("\r\n");
                if (pos == std::string::npos)
                    return ParseResult::INCOMPLETE;

                std::string line = buffer.substr(0, pos);
                buffer.erase(0, pos + 2); // remove processed line

                if (!parseStartLine(line))
                {
                    parseState = ParseState::ERROR;
                    return ParseResult::ERROR;
                }
                parseState = ParseState::HEADERS;
                break;
            }

            case ParseState::HEADERS:
            {
                size_t pos = buffer.find("\r\n");
                if (pos == std::string::npos)
                    return ParseResult::INCOMPLETE;

                std::string line = buffer.substr(0, pos);
                buffer.erase(0, pos + 2);

                if (line.empty() || line == "\r") // End of headers
                {
                    std::cout << "End of headers detected." << std::endl;
                    // Decide if a body is expected
                    if (method == Method::GET || method == Method::DELETE)
                    {
                        parseState = ParseState::COMPLETE; // Transition to COMPLETE state
                        //std::cout << "\nCurrent parse state: " << static_cast<int>(parseState) << " (not a Method::POST)" << std::endl;
                        return ParseResult::COMPLETE;
                    }
                    if (method == Method::POST)
                        parseState = ParseState::BODY;
                        //std::cout << "\nCurrent parse state: " << static_cast<int>(parseState) << " (Method::POST)" << std::endl;
                }

                if (!parseHeaders(line))
                {
                    parseState = ParseState::ERROR;
                    return ParseResult::ERROR;
                }
                break;
            }

            case ParseState::BODY:
            {
                auto it = headers.find("Content-Length");
                if (it != headers.end())
                {
                    size_t contentLength = std::stoi(it->second);
                    if (buffer.size() < contentLength)
                        return ParseResult::INCOMPLETE;

                    if (bodyFile.is_open())
                        bodyFile.write(buffer.data(), contentLength);
                    
                    buffer.erase(0, contentLength);
                    return ParseResult::COMPLETE;
                }

                // TODO: Handle Transfer-Encoding: chunked
                return ParseResult::ERROR;
            }

            case ParseState::ERROR:
                return ParseResult::ERROR;

            case ParseState::COMPLETE:
                return ParseResult::COMPLETE;
        }
    }
}



bool    HttpRequest::parseRequestFromCompleteBuffer()
{
    std::istringstream raw(rawRequest);
    std::string line;

    // Clear previous data
    reset();

    setBody("./http_request_body.txt");
    
    while (std::getline(raw, line))
    {
        // log the parsing state
        //std::cout << "Processing line: " << line << std::endl;
        //std::cout << "Current parse state: " << static_cast<int>(parseState) << std::endl;
        
        switch (parseState)
        {
            case ParseState::START_LINE:
                if (!parseStartLine(line))
                {
                    parseState = ParseState::ERROR;
                    return false;
                }    
                parseState = ParseState::HEADERS;
                break;
                
                case ParseState::HEADERS:  
                if (line.empty() || line == "\r") // End of headers
                {
                    std::cout << "\nEnd of headers detected." << std::endl;
                    // Skip BODY state if the method does not support a body
                    if (method == Method::GET || method == Method::DELETE)
                    {
                        parseState = ParseState::COMPLETE;
                        //std::cout << "\nCurrent parse state: " << static_cast<int>(parseState) << " (it is not Method::POST)" << std::endl;
                        return true;
                    }
                    parseState = ParseState::BODY;
                }  
                else if (!parseHeaders(line))
                {
                    parseState = ParseState::ERROR;
                    return false;
                }    
                break;
                
                case ParseState::BODY:
                std::cout << "i am in the BODY state " << std::endl;  
                if (bodyFile.is_open())
                {
                    std::cout << "Writing line to body file: " << line << std::endl;
                    bodyFile << line << "\n";
                    bodySize += line.size() + 1;
                }
                else
                {
                    std::cerr << "Body file is not open!" << std::endl;
                }
                break;
                
                default:    
                parseState = ParseState::ERROR;
                return false;
        }        
        
    } 
    //log_first_line();  
    //log_headers();

    parseState = ParseState::COMPLETE;
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




void HttpRequest::log_headers()
{
    for (const auto &header : headers)
    {
        std::cout << header.first << ": " << header.second << std::endl;
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
// std::string HttpRequest::readFile(const std::string& filePath)
// {
//     std::ifstream file(filePath);
//     if (!file.is_open())
//     {
//         return "./www/error/404.html";
//     }
//     std::stringstream buffer;
//     buffer << file.rdbuf();
//     return buffer.str();
// }

std::string HttpRequest::readFile(const std::string& filePath) const
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        return "./www/error/404.html";
    }
    std::ostringstream buffer;
    char chunk[8192];
    while (file.read(chunk, sizeof(chunk)))
    {
        buffer.write(chunk, file.gcount());
    }
    buffer.write(chunk, file.gcount()); // Write any remaining bytes
    return buffer.str();
}

bool HttpRequest::receiveReq(int client_fd)
{
	char buf[4096];
	ssize_t bytes = recv(client_fd, buf, sizeof(buf), 0);
	if (bytes <= 0)
	{
		disconnect = true;
		return (true);
	}

	rawRequest.append(buf, bytes);

	if (!header_received)
	{
		header_received = true;
		size_t header_end = rawRequest.find("\r\n\r\n");

		if (header_end != std::string::npos)
		{
			header_str = rawRequest.substr(0, header_end + 4);

			size_t cl_pos = header_str.find("Content-Length:");
			if (cl_pos != std::string::npos)
			{
				size_t value_start = header_str.find_first_not_of(" ", cl_pos + 15);
				size_t value_end = header_str.find("\r\n", value_start);
				std::string str_len = header_str.substr(value_start, value_end - value_start);
				expected_len = std::atoi(str_len.c_str());
			}
			else
				expected_len = 0;
			
			body_start = header_end + 4;
		}
	}

	if (header_received && !body_received)
	{
		size_t total_body_size = rawRequest.size() - body_start;
		if (expected_len == 0 || total_body_size >= expected_len)
		{
			body_received = true;
			isComplete = true;
		}
	}

	return (isComplete);
}

// This is moved and developed in the httpResponse.cpp file


// std::string HttpRequest::buildResponse()
// {
//     std::string status;
//     std::string content_type;
//     std::string body;

//     if(path == "/")
//     {
//         status = "HTTP/1.1 200 OK";
//         content_type = "text/html; charset=utf-8";
//         body = readFile("./www/index2.html");
//     }
//     else if (path == "/assets/images")
//     {
//         status = "HTTP/1.1 200 OK";
//         content_type = "image/svg+xml";
//         body = readFile("./www/assets/images/logo.svg");
//     }
//     else
//     {
//         status = "HTTP/1.1 404 Not Found";
//         content_type = "text/html; charset=utf-8";
//         body = readFile("./www/Error/404.html");
//     }

//     //Assemble the complete HTTP response
//     std::ostringstream response;
//     response << status << "\r\n";
//     response << "Content-Type: " << content_type << "\r\n";
//     response << "Content-Length: " << body.length() << "\r\n";
//     response << "\r\n";
//     response << body;

//     // std::cout << "---> HTTP Response:\n" << response.str() << std::endl; // print the complete HTTP response
//     return response.str();
// }
