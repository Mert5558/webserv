/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/01 17:46:59 by cmakario          #+#    #+#             */
/*   Updated: 2025/08/01 18:18:44 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/httpResponse.hpp"






std::string httpResponse::buildResponse()
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

std::string generateHttpResponse(int statusCode, const std::string& contentType, const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << getStatusText(statusCode) << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n"; // or keep-alive based on headers
    response << "\r\n";
    response << body;
    return response.str();
}