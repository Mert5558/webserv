#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include <string>
#include <map>
#include <sstream>
#include "../inc/ParseHttp.hpp"

class httpResponse
{
private:
	std::string statusCode; // e.g., "200 OK"
	std::string contentType; // e.g., "text/html"
	std::string body;        // The response body content
	std::map<std::string, std::string> headers;

public:
	// ========== OCF ==========
	httpResponse();
	httpResponse(const std::string &statusCode, const std::string &contentType, const std::string &body);
	~httpResponse();

	// ========== Setters ==========
	void setStatusCode(const std::string &statusCode);
	void setContentType(const std::string &contentType);
	void setBody(const std::string &body);
	void addHeader(const std::string &key, const std::string &value);

	// Build the final response string
	std::string buildResponse(const HttpRequest& request) const;

	// ====== Getters ==========
	std::string getStatusCode() const;
	std::string getContentType() const;
	std::string getBody() const;
	std::map<std::string, std::string> getHeaders() const;
};

#endif // !HTTPRESPONSE_HPP
