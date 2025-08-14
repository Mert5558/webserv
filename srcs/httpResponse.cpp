#include "../inc/httpResponse.hpp"
#include "../inc/httpResponse.hpp"
#include "../inc/Webserv.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <sstream>
#include <fstream>
#include <cstring>
#include <algorithm>


// ================== Helper Methods ==================
void httpResponse::buildHeadersOnce()
{
	if (headersBuilt)
	{
		return;
	}

	std::ostringstream ss;
	ss << "HTTP/1.1 " << statusCode << "\r\n";

	// If file streaming: Content-Length is fileSize, else it's body.size()
	if (useFile)
	{
		ss << "Content-Type: " << contentType << "\r\n";
		ss << "Content-Length: " << static_cast<unsigned long long>(fileSize) << "\r\n";
	}
	else
	{
		ss << "Content-Type: " << contentType << "\r\n";
		ss << "Content-Length: " << body.size() << "\r\n";
	}

	ss << "Connection: close\r\n";

	// Add extra headers if any (avoid duplicates)
	for (std::map<std::string,std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		if (it->first != "Content-Type" && it->first != "Content-Length" && it->first != "Connection")
		{
			ss << it->first << ": " << it->second << "\r\n";
		}
	}

	ss << "\r\n";
	headerBytes = ss.str();
	headerOffset = 0;
	headersBuilt = true;
}

static bool ends_with(const std::string &str, const char *suf)
{
	size_t lt = std::strlen(suf);
	if (str.size() < lt)
		return false;
	return str.compare(str.size() - lt, lt, suf) == 0;
}

std::string httpResponse::guessType(const std::string &path)
{
	if (ends_with(path, ".html"))
		return "text/html";
	if (ends_with(path, ".css"))
		return "text/css";
	if (ends_with(path, ".js"))
		return "application/javascript";
	if (ends_with(path, ".png"))
		return "image/png";
	if (ends_with(path, ".jpg") || ends_with(path, ".jpeg"))
		return "image/jpeg";
	if (ends_with(path, ".gif"))
		return "image/gif";
	return "text/plain";
}

// Check if the given path is a regular file and get its size
bool httpResponse::statRegularFile(const std::string &path, off_t &outSize)
{
	struct stat fileInfo;

	// Try to get information about the file
	if (stat(path.c_str(), &fileInfo) != 0)
	{
		// the file doesn't exist or can't be accessed
		return false;
	}

	// Check if it's a regular file (not a folder, device, or special file)
	if (!S_ISREG(fileInfo.st_mode))
	{
		return false;
	}

	outSize = fileInfo.st_size;
	return true;
}

// Open a file in read-only mode
int httpResponse::openReadOnly(const std::string &path)
{
	return ::open(path.c_str(), O_RDONLY);
}

static std::string normalizePath(const std::string& path)
{
	std::vector<std::string> stack;
	std::string segment;

	// Treat empty as "/"
	std::string p = path.empty() ? "/" : path;

	// Force it to behave as absolute for parsing
	size_t i = 0;
	if (p[0] != '/') {
		p = "/" + p;
	}

	// Split by '/'
	for (; i <= p.size(); ++i) {
		if (i == p.size() || p[i] == '/') {
			if (!segment.empty()) {
				if (segment == ".") {
					// skip
				} else if (segment == "..") {
					if (!stack.empty()) stack.pop_back();
					// else: ignore extra leading ".." (don't climb above root)
				} else {
					stack.push_back(segment);
				}
				segment.clear();
			}
		} else {
			segment.push_back(p[i]);
		}
	}

	// Rebuild
	if (stack.empty()) return "/";

	std::string out;
	for (size_t j = 0; j < stack.size(); ++j) {
		out += "/";
		out += stack[j];
	}
	return out;
}

static std::string safeJoin(const std::string& root, const std::string& cleaned)
{
	if (cleaned == "/")
		return root;
	if (!root.empty() && root.back() == '/')
		return root + cleaned.substr(1);
	return root + cleaned;
}



// ================== Costructors/Destructor ==================
httpResponse::httpResponse()
: statusCode("200 OK"),
  contentType("text/plain"),
  headersBuilt(false),
  headerOffset(0),
  useFile(false),
  fileFd(-1),
  fileSize(0),
  fileOffset(0),
  stageOffset(0),
  done(false)
{}

httpResponse::httpResponse(const std::string &status, const std::string &type, const std::string &bodyStr)
: statusCode(status),
  contentType(type),
  body(bodyStr),
  headersBuilt(false),
  headerOffset(0),
  useFile(false),
  fileFd(-1),
  fileSize(0),
  fileOffset(0),
  stageOffset(0),
  done(false)
{}

httpResponse::~httpResponse()
{
	if (fileFd != -1)
	{
		::close(fileFd);
	}
}

// ================== Setters ==================

void httpResponse::setStatusCode(const std::string &status)
{
	statusCode = status;
}

void httpResponse::setContentType(const std::string &type)
{
	contentType = type;
}

void httpResponse::setBody(const std::string &bodyStr)
{
	body = bodyStr;
}

void httpResponse::addHeader(const std::string &key, const std::string &value)
{
	headers[key] = value;
}


// ================== Build full string response ==================

std::string httpResponse::buildResponse() const
{
	std::ostringstream ss;
	ss << "HTTP/1.1 " << statusCode << "\r\n";

	// base headers
	ss << "Content-Type: " << contentType << "\r\n";
	ss << "Content-Length: " << body.size() << "\r\n";
	ss << "Connection: close\r\n";

	// extra headers
	for (std::map<std::string,std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		if (it->first != "Content-Type" && it->first != "Content-Length" && it->first != "Connection")
		{
			ss << it->first << ": " << it->second << "\r\n";
		}
	}

	ss << "\r\n";
	ss << body;
	return ss.str();
}





bool httpResponse::isDone() const
{
	return done;
}

