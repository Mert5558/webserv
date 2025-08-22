/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkaratsi <kkaratsi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 21:33:30 by kkaratsi          #+#    #+#             */
/*   Updated: 2025/08/18 20:50:47 by kkaratsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Webserv.hpp"
#include "../inc/HttpRequest.hpp"
#include "../inc/ParseConfig.hpp"


HttpRequest::HttpRequest()
  : method(Method::INVALID),
    version(Version::INVALID),
    headers(),
    path(),
    rawRequest(),
    parseState(ParseState::START_LINE),
    parseResult(ParseResult::INCOMPLETE),
    content_length(0),

    bodyFile(),
    bodySize(0),
    bodyFilePath(),
    disconnect(false)	
{
  // finish init the variables
}

HttpRequest::HttpRequest(const HttpRequest &copy)
	: method(copy.method),
	  version(copy.version),
	  headers(copy.headers),
	  path(copy.path),
	  rawRequest(copy.rawRequest),
	  parseState(copy.parseState),
	  parseResult(copy.parseResult),
	  content_length(copy.content_length),
	  chunk_remain_bytes(copy.chunk_remain_bytes),
	  bodySize(copy.bodySize),
	  bodyFilePath(copy.bodyFilePath),
	  disconnect(copy.disconnect)
{

}

HttpRequest &HttpRequest::operator=(const HttpRequest &copy)
{
    if (this != &copy)
    {
        method = copy.method;
        path = copy.path;
        version = copy.version;
        headers = copy.headers;
        rawRequest = copy.rawRequest;

        parseState = copy.parseState;
        parseResult = copy.parseResult;
        content_length = copy.content_length;
        chunk_remain_bytes = copy.chunk_remain_bytes;

        bodySize = copy.bodySize;
        bodyFilePath = copy.bodyFilePath;

        disconnect = copy.disconnect;
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



// ======================================================
// Setter                           
// ======================================================

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



// ======================================================
// Getter                           
// ======================================================

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

std::string HttpRequest::getUploadedFilename() const
{
	return (uploadedFilename);
}


// ======================================================
// Parsing functions for Header block and request line                          
// ======================================================

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

        for (size_t i = 0; i < key.size(); ++i)
		{
			key[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(key[i])));
		}
        
        headers[key] = value;       
    }
    return true;
    
}


// ======================================================
// Helper functions for handle the chunked body                            
// ======================================================

ParseResult HttpRequest::handleChunkSize(std::string &rawRequest)
{
    size_t eol = rawRequest.find("\r\n");
    if (eol == std::string::npos)
        return ParseResult::INCOMPLETE;

    std::string sizeLine = rawRequest.substr(0, eol);
    rawRequest.erase(0, eol + 2);

    size_t sc = sizeLine.find(';');
    if (sc != std::string::npos) sizeLine = sizeLine.substr(0, sc);

    chunk_remain_bytes = static_cast<size_t>(std::strtoul(sizeLine.c_str(), NULL, 16));
    if (chunk_remain_bytes == 0)
        parseState = ParseState::CHUNK_CRLF;
    else
        parseState = ParseState::CHUNK_DATA;

    return ParseResult::INCOMPLETE;
}


ParseResult HttpRequest::handleChunkData(std::string &rawRequest)
{
    if (rawRequest.size() < chunk_remain_bytes + 2)
        return ParseResult::INCOMPLETE;

    if (bodyFile.is_open())
        bodyFile.write(rawRequest.data(), chunk_remain_bytes);

    rawRequest.erase(0, chunk_remain_bytes);

    if (rawRequest.size() < 2)
        return ParseResult::INCOMPLETE;
    if (rawRequest.compare(0, 2, "\r\n") != 0)
        return ParseResult::ERROR;
    rawRequest.erase(0, 2);

    parseState = ParseState::CHUNK_SIZE;
    return ParseResult::INCOMPLETE;
}

ParseResult HttpRequest::handleChunkCRLF(std::string &rawRequest)
{
    if (rawRequest.size() < 2)
        return ParseResult::INCOMPLETE;
    if (rawRequest.compare(0, 2, "\r\n") != 0)
        return ParseResult::ERROR;
    rawRequest.erase(0, 2);

    parseState = ParseState::COMPLETE;
    return ParseResult::COMPLETE;
}


// ======================================================
// Base function for Parsing with machine state                             
// ======================================================

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
					bool chunked = false;
                    std::unordered_map<std::string,std::string>::const_iterator te = headers.find("transfer-encoding");
                    if (te != headers.end() && te->second.find("chunked") != std::string::npos)
                        chunked = true;

                    if (!bodyFile.is_open())
                        setBody("./http_request_body.txt");

                    if (chunked)
                    {
                        parseState = ParseState::CHUNK_SIZE;
                        return handleChunkSize(rawRequest); // may still be INCOMPLETE
                    }

                    if (headers.find("content-length") == headers.end())
                    {
                        std::cerr << "POST without Content-Length or chunked\n";
                        parseState = ParseState::ERROR;
                        return ParseResult::ERROR;
                    }
                    
                    parseState = ParseState::BODY;
                    std::cout << "\nCurrent parse state: " << static_cast<int>(parseState) << " (Method::POST)" << std::endl;
                }
            }

            case ParseState::BODY:
            {
                std::unordered_map<std::string, std::string>::const_iterator it = headers.find("content-length");
                if (it != headers.end())
                {
                    size_t contentLength = static_cast<size_t>(std::strtoul(it->second.c_str(), NULL, 10));
                    if (rawRequest.size() < contentLength)
                        return ParseResult::INCOMPLETE;

                    if (bodyFile.is_open())
					{
                        bodyFile.write(rawRequest.data(), contentLength);
						bodyFile.flush();
						bodySize = contentLength;
					}
                    
                    rawRequest.erase(0, contentLength);

					if (method == Method::POST)
						parseMultipartFilename(bodyFilePath);

                    parseState = ParseState::COMPLETE;
                    return ParseResult::COMPLETE;
                }
                
                return ParseResult::ERROR;
            }
            
            case ParseState::CHUNK_SIZE:
            {
                return handleChunkSize(rawRequest);
            }

            case ParseState::CHUNK_DATA:
            {
                return handleChunkData(rawRequest);
            }

            case ParseState::CHUNK_CRLF:
            {
                return handleChunkCRLF(rawRequest);
            }

            case ParseState::ERROR:
                return ParseResult::ERROR;

            case ParseState::COMPLETE:
                return ParseResult::COMPLETE;
        }
    }
}


void HttpRequest::parseMultipartFilename(const std::string &bodyFilePath)
{
    std::ifstream file(bodyFilePath, std::ios::binary);
    if (!file.is_open()) {
        uploadedFilename.clear();
        return;
    }

    std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::string boundary;
    auto it = headers.find("content-type");
    if (it != headers.end()) {
        std::string ct = it->second;
        size_t bpos = ct.find("boundary=");
        if (bpos != std::string::npos)
            boundary = "--" + ct.substr(bpos + 9);
    }
    if (boundary.empty()) {
        uploadedFilename.clear();
        return;
    }

    size_t partStart = body.find(boundary);
    if (partStart == std::string::npos) {
        uploadedFilename.clear();
        return;
    }
    partStart += boundary.size() + 2; // skip boundary and CRLF

    size_t headersEnd = body.find("\r\n\r\n", partStart);
    if (headersEnd == std::string::npos) {
        uploadedFilename.clear();
        return;
    }

    std::string partHeaders = body.substr(partStart, headersEnd - partStart);

    // Find filename in Content-Disposition
    std::string filename;
    size_t fnPos = partHeaders.find("filename=\"");
    if (fnPos != std::string::npos) {
        size_t fnEnd = partHeaders.find("\"", fnPos + 10);
        if (fnEnd != std::string::npos)
            filename = partHeaders.substr(fnPos + 10, fnEnd - (fnPos + 10));
    }
    if (filename.empty()) {
        uploadedFilename.clear();
        return;
    }

    // File data starts after headers
    size_t dataStart = headersEnd + 4;
    size_t dataEnd = body.find(boundary, dataStart);
    if (dataEnd == std::string::npos || dataEnd < 2) {
        uploadedFilename.clear();
        return;
    }
    dataEnd -= 2; // minus CRLF before boundary

    std::string fileData = body.substr(dataStart, dataEnd - dataStart);

    // Save fileData to disk
    std::ofstream ofs("./www/uploads/" + filename, std::ios::binary);
    if (!ofs.is_open()) {
        uploadedFilename.clear();
        return;
    }
    ofs.write(fileData.c_str(), fileData.size());
    ofs.close();

    uploadedFilename = filename;
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


// ======================================================
// Function for Parsing with machine state                             
// ======================================================

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


// ======================================================
// Helper functions                             
// ======================================================

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
	parseResult = ParseResult::INCOMPLETE;
	content_length = 0;
	chunk_remain_bytes = 0;
	bodySize = 0;
	bodyFilePath.clear();

	rawRequest.clear();
	disconnect = false;	
}

std::string_view    HttpRequest::trim(std::string_view str)
{
    size_t wspace_start = 0;
    while (wspace_start < str.size() && isspace(static_cast<unsigned char>(str[wspace_start]))) wspace_start++;
    size_t wspace_end = str.size();
    while (wspace_end > wspace_start && isspace(static_cast<unsigned char>(str[wspace_end - 1]))) wspace_end--;
    return str.substr(wspace_start, wspace_end - wspace_start);
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

// bool    HttpRequest::receiveReq(int client_fd)
// {
//     char buf[4096];
//     while (true)
//     {
//         ssize_t bytes = recv(client_fd, buf, sizeof(buf), 0);
//         if (bytes > 0)
//         {
//             rawRequest.append(buf, buf + bytes);
//             continue;
//         }
//         else if (bytes == 0)
//         {
//             disconnect = true;
//             return false;
//         }
//         else
//         {
//             if (errno == EAGAIN || errno == EWOULDBLOCK)
//             {
//                 break;
//             }
//             if (errno == EINTR)
// 			{
// 				continue;
// 			}
//             return false;
//         }
//     }
//     return true;
// }