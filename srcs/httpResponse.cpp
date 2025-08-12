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
// std::string httpResponse::buildResponse() const {
// 	std::ostringstream response;

// 	response << "HTTP/1.1 " << statusCode << "\r\n";
// 	response << "Content-Type: " << contentType << "\r\n";
// 	response << "Content-Length: " << body.length() << "\r\n";

// 	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
// 		response << it->first << ": " << it->second << "\r\n";

// 	response << "\r\n" << body;

// 	return response.str();
// }





std::string httpResponse::buildResponse(const HttpRequest& request) const
{
    std::string status;
    std::string content_type;
    std::string body;

    if(request.getPath() == "/")
    {
        status = "HTTP/1.1 200 OK";
        content_type = "text/html; charset=utf-8";
        body = request.readFile("./www/index2.html");
    }
    else if (request.getPath() == "/assets/images") // Fixed 'path' to 'request.getPath()'
    {
        status = "HTTP/1.1 200 OK";
        content_type = "image/svg+xml";
        body = request.readFile("./www/assets/images/logo.svg");
    }
    else
    {
        status = "HTTP/1.1 404 Not Found";
        content_type = "text/html; charset=utf-8";
        body = request.readFile("./www/Error/404.html");
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
