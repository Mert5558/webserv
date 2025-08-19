#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include "HttpRequest.hpp"
#include <string>
#include <map>
#include "InitConfig.hpp"

#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <filesystem>


class HttpResponse
{
private:
	// Classic fields (as you already had)
	std::string								statusCode;     // e.g. "200 OK"
	std::string								contentType;    // e.g. "text/html"
	std::string								body;           // response body
	std::map<std::string, std::string>		headers;        // extra headers

	// ===== Helpers (private) =====
	static std::string	slurpFile(const std::string &path);
	static std::string	guessType(const std::string &path);
	static bool			isRegular(const std::string &path, off_t &size);
	static bool			isDirectory(const std::string &path);
	static bool			fileExists(const std::string &path);
	static std::string	defaultErrorBody(int code, const std::string &message);

	// Safe path joining
	static std::string	joinUnderRoot(const std::string &root, const std::string &target);
	static std::string	makeAbsolute(const std::string &path);
	static bool			isUnderRootAbs(const std::string &absPath, const std::string &absRoot);
	static std::string	percentDecode(const std::string &in);


public:
	//========== OCF ==========
	HttpResponse();
	~HttpResponse();

	// Your existing builder
	std::string			buildResponse() const;

	// *** DO NOT CHANGE THIS SIGNATURE ***
	void				prepare(const HttpRequest &req, const InitConfig *server);

	// Optional setters (if you use them elsewhere)
	void				setStatusCode(const std::string &sc) { statusCode = sc; }
	void				setContentType(const std::string &ct) { contentType = ct; }
	void				setBody(const std::string &b) { body = b; }
	void				addHeader(const std::string &k, const std::string &v) { headers[k] = v; }
};

#endif