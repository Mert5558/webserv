/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkaratsi <kkaratsi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 21:33:30 by kkaratsi          #+#    #+#             */
/*   Updated: 2025/08/15 16:39:26 by kkaratsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Webserv.hpp"
#include "../inc/HttpRequest.hpp"
#include "../inc/ParseConfig.hpp"

// ParseConfig trimed;

// HttpRequest::HttpRequest() : bodySize(0), parseState(ParseState::START_LINE)
// {
//     this->method = Method::INVALID;
//     this->path = "";
//     this->version = Version::INVALID;
//     this->headers = {};
// }

// HttpRequest::HttpRequest(const HttpRequest &copy)
// {
//     method = copy.method;
//     path = copy.path;
//     version = copy.version;
//     headers = copy.headers;
// }


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
        

HttpRequest::HttpRequest()
  : method(Method::INVALID),
    version(Version::INVALID),
    headers(),
    path(),
    parseState(ParseState::START_LINE),
    parseResult(ParseResult::INCOMPLETE),
    content_length(0),
    is_chunked(false),
    expected_chunk_size(0),
    bodyFile(),
    bodySize(0),
    bodyFilePath()
{
  // finish init the variables
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

// void HttpRequest::setHeaders(const std::unordered_map<std::string, std::string> &headers)
// {
//     this->headers = headers;
// }



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


void    HttpRequest::reset()
{    
    method = Method::INVALID;
    path.clear();
    version = Version::INVALID;
    headers.clear();
    if (bodyFile.is_open())
    {
        bodyFile.close();
    }
    parseState = ParseState::START_LINE;

	rawRequest.clear();  // < -- mert
	header_str.clear(); //< --- mert
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

std::string_view HttpRequest::trim(std::string_view str)
{
    size_t wspace_start = 0;
    while (wspace_start < str.size() && isspace(static_cast<unsigned char>(str[wspace_start]))) wspace_start++;
    size_t wspace_end = str.size();
    while (wspace_end > wspace_start && isspace(static_cast<unsigned char>(str[wspace_end - 1]))) wspace_end--;
    return str.substr(wspace_start, wspace_end - wspace_start);
}

bool    HttpRequest::parseHeadersBlock(const std::string &headerBlocks)
{
    std::istringstream  stream(headerBlocks);
    std::string         line;
    
    while(std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        
        size_t  delimiterPos = line.find(':');
        if (delimiterPos == std::string::npos) continue; // skip invalid lines
        
        std::string key = std::string(trim(line.substr(0, delimiterPos)));
        std::string value = std::string(trim(line.substr(delimiterPos + 1)));

        headers[key] = value;       
    }
    return true;
    
}



ParseResult HttpRequest::parse()
{
    while (true)
    {
        // debug state of parsing
        std::cout << "Current parse state: " << static_cast<int>(parseState) << std::endl;
        
        switch (parseState)
        {
            case ParseState::START_LINE:
            {
                size_t pos = rawRequest.find("\r\n");
                if (pos == std::string::npos)
                    return ParseResult::INCOMPLETE;

                std::string line = rawRequest.substr(0, pos);
                rawRequest.erase(0, pos + 2); // remove processed line

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
                size_t pos = rawRequest.find("\r\n\r\n");
                if (pos == std::string::npos)
                    return ParseResult::INCOMPLETE;

                std::string headersBlock = rawRequest.substr(0, pos);
                rawRequest.erase(0, pos + 4);

                if (!parseHeadersBlock(headersBlock))
                {
                    parseState = ParseState::ERROR;
                    return ParseResult::ERROR;
                }
                std::cout << "End of headers detected." << std::endl;
                // Decide if a body is expected
                if (method == Method::GET || method == Method::DELETE)
                {
                    parseState = ParseState::COMPLETE; // Transition to COMPLETE state
                    std::cout << "\nCurrent parse state: " << static_cast<int>(parseState) << " COMPLETE (not a Method::POST)" << std::endl;
                    return ParseResult::COMPLETE;
                }
                if (method == Method::POST)
                {
                    parseState = ParseState::BODY;
                    std::cout << "\nCurrent parse state: " << static_cast<int>(parseState) << " (Method::POST)" << std::endl;
                }
                break;
            }

            case ParseState::BODY:
            {
                auto it = headers.find("content-length");
                if (it != headers.end())
                {
                    size_t contentLength = std::stoi(it->second);
                    if (rawRequest.size() < contentLength)
                        return ParseResult::INCOMPLETE;

                    if (bodyFile.is_open())
                        bodyFile.write(rawRequest.data(), contentLength);
                    
                    rawRequest.erase(0, contentLength);
                    return ParseResult::COMPLETE;
                }
                
                // TODO: Handle Transfer-Encoding: chunked
                return ParseResult::ERROR;
            }
            
            case ParseState::CHUNK_SIZE:
            {
                //return handleChunkSize(rawRequest);
            }

            case ParseState::CHUNK_DATA:
            {
                //return handleChunkData(rawRequest);
            }

            case ParseState::CHUNK_CRLF:
            {
                //return handleChunkCRLF(rawRequest);
            }

            case ParseState::ERROR:
                return ParseResult::ERROR;

            case ParseState::COMPLETE:
                return ParseResult::COMPLETE;
        }
    }
}



// ================== NEED TO SEE =====================
// bool    HttpRequest::parseRequestFromCompleteBuffer()
// {
//     std::istringstream raw(rawRequest);
//     std::string line;

//     // Clear previous data
//     reset();

//     setBody("./http_request_body.txt");
    
//     while (std::getline(raw, line))
//     {
//         // log the parsing state
//         //std::cout << "Processing line: " << line << std::endl;
//         //std::cout << "Current parse state: " << static_cast<int>(parseState) << std::endl;
        
//         switch (parseState)
//         {
//             case ParseState::START_LINE:
//                 if (!parseStartLine(line))
//                 {
//                     parseState = ParseState::ERROR;
//                     return false;
//                 }    
//                 parseState = ParseState::HEADERS;
//                 break;
                
//                 case ParseState::HEADERS:  
//                 if (line.empty() || line == "\r") // End of headers
//                 {
//                     std::cout << "\nEnd of headers detected." << std::endl;
//                     // Skip BODY state if the method does not support a body
//                     if (method == Method::GET || method == Method::DELETE)
//                     {
//                         parseState = ParseState::COMPLETE;
//                         //std::cout << "\nCurrent parse state: " << static_cast<int>(parseState) << " (it is not Method::POST)" << std::endl;
//                         return true;
//                     }
//                     parseState = ParseState::BODY;
//                 }  
//                 else if (!parseHeaders(line))
//                 {
//                     parseState = ParseState::ERROR;
//                     return false;
//                 }    
//                 break;
                
//                 case ParseState::BODY:
//                 std::cout << "i am in the BODY state " << std::endl;  
//                 if (bodyFile.is_open())
//                 {
//                     std::cout << "Writing line to body file: " << line << std::endl;
//                     bodyFile << line << "\n";
//                     bodySize += line.size() + 1;
//                 }
//                 else
//                 {
//                     std::cerr << "Body file is not open!" << std::endl;
//                 }
//                 break;
                
//                 default:    
//                 parseState = ParseState::ERROR;
//                 return false;
//         }        
        
//     } 
//     //log_first_line();  
//     //log_headers();

//     parseState = ParseState::COMPLETE;
//     return true;
// }    



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

// last attempt
// bool HttpRequest::receiveReq(int client_fd)
// {
//     char chunk[1024];
//     size_t bytes;

//     while ((bytes = recv(client_fd, chunk, sizeof(chunk), 0)) > 0)
//     {
//         if (bytes > 0)
//         recv_buffer.append(chunk, bytes);
//         return true;
//     }
//     if (bytes == 0)
//     { 
//         disconnect = true; 
//         return false;
//     }
//     if (errno == EAGAIN || errno == EWOULDBLOCK)
//     {
//         return false;
//         disconnect = true;
//     }
//     return false;
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

