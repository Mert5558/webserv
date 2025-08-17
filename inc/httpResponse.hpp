#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include "HttpRequest.hpp"
#include <string>
#include <map>
#include "InitConfig.hpp"

class httpResponse
{
private:
	// Classic fields
	std::string								statusCode;
	std::string								contentType;
	std::string								body;
	std::map<std::string, std::string>		headers;

	// File streaming fields
	static std::string slurpFile(const std::string &path);
	static std::string guessType(const std::string &path);
	static std::string safeJoin(const std::string &root, const std::string &target);

public:	
	// Constructors and Destructor
	httpResponse();
	~httpResponse();

	// Setters
	void setStatusCode(const std::string &status);
	void setContentType(const std::string &type);
	void setBody(const std::string &bodyStr);
	void addHeader(const std::string &key, const std::string &value);

	// Build full string response
	std::string buildResponse() const;

	// Builds a simple static file response based on request target and server root.
	void prepare(const HttpRequest &req, const InitConfig *server);
};

#endif // !HTTPRESPONSE_HPP