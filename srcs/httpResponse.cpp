#include "../inc/httpResponse.hpp"
#include "../inc/httpResponse.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <cstring>


// HTTP server must send exactly the bytes on disk—no hidden changes.
// This means we need to read files in binary mode, and handle '\0' bytes correctly.
std::string httpResponse::slurpFile(const std::string &path)
{
	// We'll put the file bytes here.
	std::string data;

	// 1) Open the file in BINARY mode
	std::ifstream in;
	in.open(path.c_str(), std::ios::in | std::ios::binary);

	// 2) If the file didn't open, return empty string.
	if (!in.is_open())
	{
		std::cerr << "[slurpFile] Could not open file: " << path << std::endl;
		return data; // empty
	}

	// 3) Read the file in fixed-size chunks
	const std::size_t CHUNK = 8192; // 8 KB
	char buffer[CHUNK];

	while (true)
	{
		//read up to CHUNK bytes.
		in.read(buffer, static_cast<std::streamsize>(CHUNK));

		// Get the number of bytes actually read.
		std::streamsize got = in.gcount();

		// Append it to the output string.
		if (got > 0)
		{
			// append(ptr, length) keeps binary data intact (including '\0' bytes).
			data.append(buffer, static_cast<std::size_t>(got));
		}

		// At end-of-file, we're done.
		if (in.eof())
		{
			break;
		}

		if (in.fail())
		{
			std::cerr << "[slurpFile] Read error while reading: " << path << std::endl;
			data.clear(); // to indicate failure
			break;
		}
	}

	// 4) Close the file explicitly
	in.close();

	// 5) Return all bytes as a string.
	return data;
}
// Check if the string ends with the given suffix
static bool ends_with(const std::string &str, const char *suf)
{
	size_t lt = std::strlen(suf);
	if (str.size() < lt)
		return false;
	return str.compare(str.size() - lt, lt, suf) == 0;
}

// Guess the content type based on the file extension
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

// Secure path normalization
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

// Prepare a response based on the request and server root
std::string httpResponse::safeJoin(const std::string& root, const std::string& cleaned)
{
	if (cleaned == "/")
		return root;
	if (!root.empty() && root.back() == '/')
		return root + cleaned.substr(1);
	return root + cleaned;
}

// ================== Costructors/Destructor ==================
httpResponse::httpResponse() : statusCode("200 OK"), contentType("text/plain") {}
httpResponse::~httpResponse() {}

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

	// extra headers (skip duplicates)
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

// Check if the given path is a regular file and get its size
static bool isRegular(const std::string &path, off_t &outSize)
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

// Check if the given path is a directory
static bool isDirectory(const std::string &path)
{
	struct stat info;

	if (stat(path.c_str(), &info) != 0)
	{
		// cannot stat (doesn't exist or no permission)
		return false;
	}

	if (S_ISDIR(info.st_mode))
	{
		return true;
	}

	return false;
}

// ================== Prepare response based on request and server root ==================
void httpResponse::prepare(const HttpRequest &request, const InitConfig *server)
{
	// 1) Decide server root and index filename (no ternaries)
	std::string serverRoot;
	std::string indexFilename;

	if (server != NULL)
	{
		serverRoot = server->getRoot();
		indexFilename = server->getIndex();
	}
	else
	{
		serverRoot = ".";
		indexFilename = "index.html";
	}

	// 2) Normalize the requested target path
	//    - Treat empty as "/"
	std::string requestedTarget;

	if (request.target.empty())
	{
		requestedTarget = "/";
	}
	else
	{
		requestedTarget = request.target;
	}

	// 3) If the target ends with a slash, serve the directory index
	//    e.g., "/docs/" -> "/docs/index.html"
	if (!requestedTarget.empty())
	{
		char lastChar = requestedTarget[requestedTarget.size() - 1];
		if (lastChar == '/')
		{
			requestedTarget += indexFilename;
		}
	}

	// 4) Clean the target (collapse ".", "..", extra slashes)
	//    Your normalizePath() returns absolute-ish like "/a/b"
	std::string normalizedTarget = normalizePath(requestedTarget);

	// 5) Join safely using YOUR safeJoin(root, cleaned) that RETURNS a string
	std::string resolvedPath = safeJoin(serverRoot, normalizedTarget);

	// 6) If this is a directory (e.g., "/docs" without trailing slash),
	//    try appending "/index.html" as a simple rule.
	if (isDirectory(resolvedPath))
	{
		std::string dirTarget = normalizedTarget;

		if (dirTarget.empty() || dirTarget[dirTarget.size() - 1] != '/')
		{
			dirTarget += "/";
		}
		dirTarget += indexFilename;

		// Re-normalize and re-join
		dirTarget = normalizePath(dirTarget);
		resolvedPath = safeJoin(serverRoot, dirTarget);
	}

	// 7) If it's a regular file, read and return 200 OK (empty files are valid!)
	off_t fileSize = 0;
	if (isRegular(resolvedPath, fileSize))
	{
		std::string fileData = slurpFile(resolvedPath);

		setStatusCode("200 OK");
		setContentType(guessType(resolvedPath)); // make sure this has a sensible default
		setBody(fileData);
		return;
	}

	// 8) Fallback: Not found
	setStatusCode("404 Not Found");
	setContentType("text/plain");
	setBody("Not Found\n");
}



