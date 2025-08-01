/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/01 17:46:59 by cmakario          #+#    #+#             */
/*   Updated: 2025/08/01 19:59:52 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/httpResponse.hpp"

httpResponse::httpResponse() {}

httpResponse::httpResponse(const std::string &statusCode, const std::string &contentType, const std::string &body)
	: statusCode(statusCode), contentType(contentType), body(body) {}

httpResponse::~httpResponse() {}

// Setters
void httpResponse::setStatusCode(const std::string &statusCode) {
	this->statusCode = statusCode;
}

void httpResponse::setContentType(const std::string &contentType) {
	this->contentType = contentType;
}

void httpResponse::setBody(const std::string &body) {
	this->body = body;
}

void httpResponse::addHeader(const std::string &key, const std::string &value) {
	headers[key] = value;
}

// Getters
std::string httpResponse::getStatusCode() const { return statusCode; }
std::string httpResponse::getContentType() const { return contentType; }
std::string httpResponse::getBody() const { return body; }
std::map<std::string, std::string> httpResponse::getHeaders() const { return headers; }

// Final response builder
std::string httpResponse::buildResponse() const {
	std::ostringstream response;

	response << "HTTP/1.1 " << statusCode << "\r\n";
	response << "Content-Type: " << contentType << "\r\n";
	response << "Content-Length: " << body.length() << "\r\n";

	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		response << it->first << ": " << it->second << "\r\n";

	response << "\r\n" << body;

	return response.str();
}





// std::string httpResponse::buildResponse()
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
