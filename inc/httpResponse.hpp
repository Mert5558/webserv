#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#pragma once

#include "Webserv.hpp"
#include "ParseHttp.hpp"
#include <string>
#include <map>
#include <vector>

class httpResponse
{
private:
	// Classic fields
	std::string								statusCode;
	std::string								contentType;
	std::string								body;
	std::map<std::string, std::string>		headers;
	// Streaming state
	bool									headersBuilt;
	std::string								headerBytes;
	size_t									headerOffset;
	bool									useFile;
	int										fileFd;
	off_t									fileSize;
	off_t									fileOffset;
	std::string								stage;        // staging buffer for file reads
	size_t									stageOffset;
	bool									done;

	void buildHeadersOnce();
	static std::string guessType(const std::string &path);
	static bool statRegularFile(const std::string &path, off_t &outSize);
	static int openReadOnly(const std::string &path);
	static std::string safeJoin(const std::string &root, const std::string &target);

public:
	// Constructors and Destructor
	httpResponse();
	httpResponse(const std::string &status, const std::string &type, const std::string &bodyStr);
	~httpResponse();

	// Setters
	void setStatusCode(const std::string &status);
	void setContentType(const std::string &type);
	void setBody(const std::string &bodyStr);
	void addHeader(const std::string &key, const std::string &value);

	// Build full string response
	std::string buildResponse() const;

	// Plan a response from request (GET serves files under serverRoot, POST echoes size)
	void prepare(const HttpRequest &req, const std::string &serverRoot);
	// Send as much as possible (non-blocking). Returns true if any progress was made.
	bool sendStep(int sockfd);
	// Check if the response is fully sent
	bool isDone() const;
};

#endif // !HTTPRESPONSE_HPP