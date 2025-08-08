/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseHttp.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 15:19:17 by kkaratsi          #+#    #+#             */
/*   Updated: 2025/08/08 11:57:29 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

/* Useful link ----> https://datatracker.ietf.org/doc/html/rfc7231#section-1.1 */

/* 
	---Common HTTP Headers
	Request Headers: These are sent by the client (browser or application) to provide information about the request.

	- Host: Specifies the domain name of the server (e.g., Host: example.com).
	- User-Agent: Identifies the client software making the request (e.g., User-Agent: Mozilla/5.0).
	- Accept: Indicates the media types the client can process (e.g., Accept: text/html, application/json).
	- Accept-Language: Specifies the preferred language for the response (e.g., Accept-Language: en-US).
	- Accept-Encoding: Indicates the encoding formats the client can handle (e.g., Accept-Encoding: gzip, deflate).
	- Connection: Controls whether the connection should be kept alive (e.g., Connection: keep-alive).
	- Content-Type: Specifies the media type of the request body (e.g., Content-Type: application/json).
	- Content-Length: Indicates the size of the request body in bytes (e.g., Content-Length: 123).
	- Authorization: Contains credentials for authentication (e.g., Authorization: Basic <credentials> or Bearer <token>).
	- Cookie: Sends cookies to the server (e.g., Cookie: session_id=abc123).
	
	---Custom Headers: You can define custom headers for specific use cases. These typically start with X- (e.g., X-Custom-Header: value).

*/

class	HttpRequest
{
	private:
		std::string										method;		// Represents a single HTTP method like "GET" or "POST"
		std::string										path;		// The requested resource path (e.g., "/index.html" or "/api/data").
		std::string										version;	// The HTTP version used in the request (e.g., "HTTP/1.1").
		std::string										body;		// The body of the HTTP request, typically used for POST or PUT requests.
		std::vector<std::pair<std::string, std::string>> headers;	// A collection of HTTP headers as key-value pairs (e.g., "Content-Type: application/json").	

	public:
		HttpRequest();
		HttpRequest(const HttpRequest &copy);
		HttpRequest &operator=(const HttpRequest &copy);
		~HttpRequest();

		void setMethod(const std::string &method);
		void setPath(const std::string &path);
		void setVersion(const std::string &version);
		void setBody(const std::string &body);
		void setHeaders(const std::vector<std::pair<std::string, std::string>> &headers);

		std::string	getMethod() const;
		std::string	getPath() const;
		std::string getVersion() const;
		std::string	getBody() const;
		std::vector<std::pair<std::string, std::string>> getHeaders() const;
																			
		bool parseRequest(const std::string &rawRequest);
		bool isValidMethod();
		bool isValidVersion();
		bool isValidPath();
		
		std::string receiveRequest(int client_fd);
		void log_headers(const std::vector<std::pair<std::string, std::string>> &headers);
		void log_first_line();

		// std::string buildResponse();
		std::string readFile(const std::string& filePath);
	};

std::ostream &operator<<(std::ostream &os, const HttpRequest &request);