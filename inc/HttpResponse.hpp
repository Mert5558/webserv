#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include "HttpRequest.hpp"
#include <string>
#include <map>
#include "InitConfig.hpp"
#include <string>
#include <map>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <vector>




class HttpResponse
{
private:
	// Classic fields (as you already had)
	std::string								statusCode;
	std::string								contentType;
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

	// Error pages handling
	static std::string	loadConfiguredErrorPage(int code, const InitConfig *server);
	// Autoindex HTML generation
	static std::string	buildAutoindexHtml(const std::string &webRoot,
		const std::string &absDir,
		const std::string &requestPath);
	// Render error response
	
	public:
	//========== OCF ==========
	void				renderError(int code, const std::string &reason, const InitConfig *server);
	HttpResponse();
	~HttpResponse();

	// Builder of raw HTTP response
	std::string			buildResponse() const;
	
	// Prepare response based on the request and server configuration
	void				prepare(const HttpRequest &req, InitConfig *server);

	// Setters
	void				setStatusCode(const std::string &sc);
	void				setContentType(const std::string &ct);
	void				setBody(const std::string &b) ;
	void				addHeader(const std::string &k, const std::string &v) ;
};

#endif