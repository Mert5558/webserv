#include "../inc/HttpResponse.hpp"
#include "../inc/Location.hpp"
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <iostream>
#include <limits.h>		// PATH_MAX
#include <ostream>
#include <unistd.h>		// getcwd, realpath
#include <sys/stat.h>	// stat
#include <cstring>		// std::strncmp, std::strlen
#include <cstdlib>		// realpath decl on some libcs


enum HttpMethod {
	GET = 0,
	POST = 1,
	DELETE = 2
};


// ========== Constructor / Destructor ==========

HttpResponse::HttpResponse() : statusCode("200 OK"), contentType("text/plain"), body("") {}

HttpResponse::~HttpResponse() {}

// ========== Private helpers ==========

std::string HttpResponse::slurpFile(const std::string &path)
{
	std::ifstream ifs(path.c_str(), std::ios::in | std::ios::binary);
	if (!ifs)
	{
		return "";
	}
	std::ostringstream ss;
	ss << ifs.rdbuf();
	return ss.str();
}

std::string HttpResponse::guessType(const std::string &path)
{
	std::string::size_type dot = path.rfind('.');
	if (dot == std::string::npos)
	{
		return "application/octet-stream";
	}
	std::string ext = path.substr(dot + 1);
	for (size_t i = 0; i < ext.size(); ++i)
	{
		ext[i] = static_cast<char>(std::tolower(ext[i]));
	}

	if (ext == "html" || ext == "htm")
	{
		return "text/html; charset=utf-8";
	}
	if (ext == "txt")
	{
		return "text/plain; charset=iso-8859-1";
	}
	if (ext == "css")
	{
		return "text/css; charset=iso-8859-1";
	}
	if (ext == "js")
	{
		return "application/javascript";
	}
	if (ext == "png")
	{
		return "image/png";
	}
	if (ext == "jpg" || ext == "jpeg")
	{
		return "image/jpeg";
	}
	if (ext == "gif")
	{
		return "image/gif";
	}

	return "application/octet-stream";
}

bool HttpResponse::isRegular(const std::string &path, off_t &size)
{
	struct stat st;
	if (stat(path.c_str(), &st) == -1)
	{
		return false;
	}
	if (S_ISREG(st.st_mode))
	{
		size = st.st_size;
		return true;
	}
	return false;
}

bool HttpResponse::isDirectory(const std::string &path)
{
	struct stat st;
	if (stat(path.c_str(), &st) == -1)
	{
		return false;
	}
	return S_ISDIR(st.st_mode);
}

bool HttpResponse::fileExists(const std::string &path)
{
	struct stat st;
	return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

std::string HttpResponse::defaultErrorBody(int code, const std::string &message)
{
	std::ostringstream html;
	html << "<!DOCTYPE html>\n"
	     << "<html>\n"
	     << "<head><meta charset=\"utf-8\"><title>" << code << " " << message << "</title></head>\n"
	     << "<body>\n"
	     << "<h1>" << code << " " << message << "</h1>\n"
	     << "<hr><p>webserv</p>\n"
	     << "</body>\n"
	     << "</html>\n";
	return html.str();
}

// Strip query and fragment from a URL path
static std::string stripQueryFragment(const std::string &t)
{
	size_t q = t.find_first_of("?#");
	if (q == std::string::npos)
	{
		return t;
	}
	return t.substr(0, q);
}

std::string HttpResponse::joinUnderRoot(const std::string &root, const std::string &target)
{
	// Normalize: ensure target begins with '/'
	std::string t = target.empty() ? "/" : target;
	t = stripQueryFragment(t);
	if (!t.empty() && t[0] != '/')
	{
		t = "/" + t;
	}

	// Split and drop '.' and '..'
	std::vector<std::string> parts;
	std::string seg;
	for (size_t i = 0; i <= t.size(); ++i)
	{
		if (i == t.size() || t[i] == '/')
		{
			if (!seg.empty())
			{
				if (seg == ".")
				{
					// skip
				}
				else if (seg == "..")
				{
					if (!parts.empty())
					{
						parts.pop_back();
					}
				}
				else
				{
					parts.push_back(seg);
				}
				seg.clear();
			}
		}
		else
		{
			seg.push_back(t[i]);
		}
	}

	std::string out = root;
	if (!out.empty() && out[out.size() - 1] == '/')
	{
		out.erase(out.size() - 1);
	}
	for (size_t i = 0; i < parts.size(); ++i)
	{
		out += "/";
		out += parts[i];
	}

	// If request was "/" -> out is just root
	if (parts.empty())
	{
		out = root;
	}

	return out;
}

std::string HttpResponse::makeAbsolute(const std::string &path)
{
	if (!path.empty() && path[0] == '/')
	{
		return path;
	}
	if (!path.empty() && (path[0] == '.' || path.substr(0, 2) == "./"))
	{
		char buf[4096];
		if (::getcwd(buf, sizeof(buf)) == NULL)
			return (path);
		std::string base(buf);
		if (!base.empty() && base[base.size() -1] != '/')
			base += "/";
		return (base + path);
	}
	char buf[4096];
	if (::getcwd(buf, sizeof(buf)) == NULL)
	{
		return path; // fallback
	}
	std::string base(buf);
	if (!base.empty() && base[base.size() - 1] != '/')
	{
		base += "/";
	}
	return base + path;
}

// Check if the absolute path is under the root directory -- Safety Handle
bool HttpResponse::isUnderRootAbs(const std::string &absPath, const std::string &absRoot)
{
	if (absRoot.empty())
	{
		return false;
	}

	// 1) Canonicalize the root
	char rootReal[PATH_MAX];
	if (!realpath(absRoot.c_str(), rootReal))
	{
		return false;
	}

	// 2) If target exists, canonicalize it and do a safe prefix+boundary check
	struct stat st;
	if (stat(absPath.c_str(), &st) == 0)
	{
		char pathReal[PATH_MAX];
		if (!realpath(absPath.c_str(), pathReal))
		{
			return false;
		}

		size_t n = std::strlen(rootReal);
		if (std::strncmp(pathReal, rootReal, n) != 0)
		{
			return false;
		}
		return pathReal[n] == '\0' || pathReal[n] == '/';
	}

	// 3) Target does NOT exist:
	//    If absPath was built from the SAME absRoot (string)  comput in prepare(),
	//    then absPath should begin with absRoot (including any "./"). In that case,
	//    it's lexically inside the root -> let caller return 404.
	{
		const size_t nAbsRoot = absRoot.size();
		if (absPath.size() >= nAbsRoot &&
			std::strncmp(absPath.c_str(), absRoot.c_str(), nAbsRoot) == 0 &&
			(absPath.size() == nAbsRoot || absPath[nAbsRoot] == '/'))
		{
			// Quick sanity—joinUnderRoot already stripped ".."
			if (absPath.find("..") != std::string::npos)
			{
				return false;
			}
			return true; // inside root lexically -> let higher layer decide 404
		}
	}

	// 4) Fallback: find deepest existing ancestor and ensure THAT is under root
	std::string cur = absPath;
	while (!cur.empty())
	{
		struct stat st2;
		if (stat(cur.c_str(), &st2) == 0 && S_ISDIR(st2.st_mode))
		{
			char parentReal[PATH_MAX];
			if (!realpath(cur.c_str(), parentReal))
			{
				return false;
			}
			size_t n = std::strlen(rootReal);
			if (std::strncmp(parentReal, rootReal, n) != 0)
			{
				return false;
			}
			return parentReal[n] == '\0' || parentReal[n] == '/';
		}

		size_t pos = cur.rfind('/');
		if (pos == std::string::npos)
		{
			break;
		}
		if (pos == 0)
		{
			cur = "/";
			break;
		}
		cur.erase(pos);
	}

	return false;
}


std::string HttpResponse::loadConfiguredErrorPage(int code, const InitConfig *server)
{
	// server->getErrorPagePath(code)
	if (!server)
	{
		return "";
	}

	// ? Implement this based on server's API. or keep one down
	// std::string rel = server->getErrorPagePath(code);

	std::cout << "[DBG---------] Loading error page for code: " << code << std::endl;
	std::string rel;
	{
		const std::map<short, std::string> &errMap = server->getErrorPages();
		// for (std::map<short, std::string>::const_iterator it = errMap.begin(); it != errMap.end(); ++it)
		// {
		// 	std::cout << "Error Code: " << it->first << ", Page Path: " << it->second << std::endl;
		// }
		// std::cout << "[DBG------] Loading error page for code: " << code << std::endl;
		// std::cout << "[DBG------] << " << server->getErrorPages().size() << " error pages configured." << std::endl;
		std::map<short, std::string>::const_iterator it = errMap.find(code);
		if (it != errMap.end())
		{
			rel = it->second;
		}
		std::cout << "[DBG------] Error page path: " << rel << std::endl;
	}

	if (rel.empty())
	{
		return "";
	}

	// Build absolute path under root and read it
	std::string abs = rel;
	std::cout << "[DBG------] Absolute error page path: " << abs << std::endl;
	std::string absNorm = makeAbsolute(abs);
	std::cout << "[DBG------] Normalized absolute error page path: " << absNorm << std::endl;

	off_t sz = 0;
	if (!isRegular(absNorm, sz))
	{
		return "";
	}
	return slurpFile(absNorm);
}

// Build minimal HTML autoindex page
static std::string escapeHtml(const std::string &s)
{
	std::string out;
	out.reserve(s.size());
	for (size_t i = 0; i < s.size(); ++i)
	{
		switch (s[i])
		{
			case '&':
				out += "&amp;";
				break;
			case '<':
				out += "&lt;";
				break;
			case '>':
				out += "&gt;";
				break;
			case '"':
				out += "&quot;";
				break;
			case '\'':
				out += "&#39;";
				break;
			default:
				out += s[i];
		}
	}
	return out;
}

std::string HttpResponse::buildAutoindexHtml(const std::string &webRoot,
											 const std::string &absDir,
											 const std::string &requestPath)
{
	// list entries and build links relative to requestPath.
	// Security: don’t show parent links outside root.
	std::ostringstream html;
	html << "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n";
	html << "<title>Index of " << escapeHtml(requestPath) << "</title>\n";
	html << "<style>body{font-family:system-ui,Arial;padding:1rem;} a{display:block;padding:.2rem 0;}</style>\n";
	html << "</head>\n<body>\n";
	html << "<h1>Index of " << escapeHtml(requestPath) << "</h1>\n<hr>\n";

	DIR *dir = opendir(absDir.c_str());
	if (!dir)
	{
		html << "<p>Cannot open directory.</p>\n</body></html>\n";
		return html.str();
	}

	// Ensure requestPath ends with '/'
	std::string baseHref = requestPath;
	if (baseHref.empty() || baseHref[baseHref.size() - 1] != '/')
	{
		baseHref += "/";
	}

	// Parent directory link if we’re not at root
	if (absDir != makeAbsolute(webRoot))
	{
		// Compute parent URL by stripping one segment
		std::string parent = requestPath;
		if (!parent.empty() && parent[parent.size() - 1] == '/')
		{
			parent.erase(parent.size() - 1);
		}
		size_t slash = parent.find_last_of('/');
		if (slash != std::string::npos)
		{
			parent = parent.substr(0, slash + 1);
		}
		else
		{
			parent = "/";
		}
		html << "<a href=\"" << escapeHtml(parent) << "\">../</a>\n";
	}
	// (void)webRoot; // Unused in this function, but kept for consistency

	struct dirent *ent;
	std::vector<std::string> names;

	while ((ent = readdir(dir)) != NULL)
	{
		std::string name = ent->d_name;
		if (name == ".")
		{
			continue;
		}
		// include ".." only if it resolves under root
		if (name == "..")
		{
			continue;
		}
		names.push_back(name);
	}
	closedir(dir);

	std::sort(names.begin(), names.end());

	for (size_t i = 0; i < names.size(); ++i)
	{
		const std::string &n = names[i];
		std::string fullEntry = absDir;
		if (!fullEntry.empty() && fullEntry[fullEntry.size() - 1] != '/')
		{
			fullEntry += "/";
		}
		fullEntry += n;

		bool isDir = isDirectory(fullEntry);
		std::string link = baseHref + n + (isDir ? "/" : "");
		html << "<a href=\"" << escapeHtml(link) << "\">" << escapeHtml(n) << (isDir ? "/" : "") << "</a>\n";
	}

	html << "<hr>\n</body>\n</html>\n";
	return html.str();
}

void HttpResponse::renderError(int code, const std::string &reason, const InitConfig *server)
{
	std::string page = loadConfiguredErrorPage(code, server);

	// --- Set status code string ---
	switch (code)
	{
		case 404: statusCode = "404 Not Found";
			break;
		case 403: statusCode = "403 Forbidden";
			break;
		case 405: statusCode = "405 Method Not Allowed";
			break;
		case 500: statusCode = "500 Internal Server Error";
			break;
		case 413: statusCode = "413 Payload Too Large";
			break;
		case 400: statusCode = "400 Bad Request";
			break;
		default: statusCode = "500 Internal Server Error";
			break;
	}

	// --- Always HTML error response ---
	contentType = "text/html; charset=iso-8859-1";

	// --- Use configured page if available, else default tiny HTML ---
	if (!page.empty())
	{
		body = page;
	}
	else
	{
		body = defaultErrorBody(code, reason);
	}

	// No  more headers here! They 'll be added automatically by buildResponse()
}


std::string HttpResponse::buildResponse() const
{
	std::ostringstream ss;
	ss << "HTTP/1.1 " << statusCode << "\r\n";

	if (headers.find("Content-Type") == headers.end())
	{
		ss << "Content-Type: " << contentType << "\r\n";
	}
	if (headers.find("Content-Length") == headers.end())
	{
		ss << "Content-Length: " << body.size() << "\r\n";
	}
	if (headers.find("Connection") == headers.end())
	{
		ss << "Connection: close\r\n";
	}
	if (headers.find("Server") == headers.end())
	{
		ss << "Server: webserv/1.0\r\n";
	}
	
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
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

std::string HttpResponse::percentDecode(const std::string &in)
{
	std::string out;
	out.reserve(in.size());

	for (size_t i = 0; i < in.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(in[i]);

		if (c == '%' && i + 2 < in.size())
		{
			unsigned char c1 = static_cast<unsigned char>(in[i + 1]);
			unsigned char c2 = static_cast<unsigned char>(in[i + 2]);

			if (std::isxdigit(c1) && std::isxdigit(c2))
			{
				int hi = std::isdigit(c1) ? (c1 - '0') : (std::tolower(c1) - 'a' + 10);
				int lo = std::isdigit(c2) ? (c2 - '0') : (std::tolower(c2) - 'a' + 10);
				out.push_back(static_cast<char>((hi << 4) | lo));
				i += 2;
				continue;
			}
		}
		out.push_back(static_cast<char>(c));
	}
	return out;
}

void HttpResponse::prepare(const HttpRequest &req, InitConfig *server)
{
	// Debug
	// server->print();
	if (req.isTooLarge())
	{
		renderError(413, "Payload Too Large", server);
		return;
	}

	// Reset minimal defaults for each response
	statusCode = "200 OK";
	contentType = "text/plain";
	body.clear();
	headers.clear();
	
	//Find location for request path
	// std::cout << "------PATHHHHHHHHHHHHHHHHHHH----- " << req.getPath() <<std::endl;

	// ============== INVALID METHOD ===================
	if (req.getMethod() != "GET" && req.getMethod() != "POST" && req.getMethod() != "DELETE")
	{
		headers["Allow"] = "GET, POST, DELETE";
		renderError(405, "Method Not Allowed", server);
		return;
	}

	//--------------------------
	// 0) Choose location/config
	//--------------------------
	Location *loc = server->findLocationForPath(req.getPath());

	std::string serverRoot;
	std::string indexName;
	std::vector<short> allowedMethods;
	// std::string allowMethodsServerUniversal;
	bool autoIndex = false;

	// If location is found, use its settings; otherwise use server defaults
	if (loc)
	{
		std::cout << "------indise loccccc-----" << std::endl;
		serverRoot = loc->getRoot();
		indexName = loc->getIndex();
		autoIndex = loc->getAutoindex();
		allowedMethods = loc->getMethods();
	}
	else
	{
		serverRoot = server->getRoot();
		indexName = server->getIndex();
		autoIndex = server->getAutoIndex();
	}

	// allowMethodsServerUniversal = server->getAllowMethods();

	// ---------------------------
	// 1) Normalize and resolve path
	// ---------------------------

	// Location path for relative target resolution
	std::string locationPath = loc ? loc->getPath() : "";
	// Resolve target path safely 
	std::string rawTarget = req.getPath().empty() ? "/" : req.getPath();

	// Decode percent-encoding so %2e%2e etc. can't bypass normalization
	std::string decodedTarget = percentDecode(rawTarget);

	// Request path relative to the matched location prefix
	if (!locationPath.empty() && decodedTarget.find(locationPath) == 0)
	{
		decodedTarget = decodedTarget.substr(locationPath.length());
			if (decodedTarget.empty())
			decodedTarget = "/";
	}

	// Map to filesystem under serverRoot
	std::string fullFSPath = joinUnderRoot(serverRoot, decodedTarget);

	const std::string absRoot = makeAbsolute(serverRoot);
	const std::string absPath = makeAbsolute(fullFSPath);

	std::cout << "[DBG] absRoot=" << absRoot << "\n";
	std::cout << "[DBG] absPath=" << absPath << "\n";

	std::cout << "[DBG] Server root: " << serverRoot << " for server at " << server->getPort() << std::endl;

	if (!isUnderRootAbs(absPath, absRoot))
	{
		renderError(404, "Not Found", server);
		return;
	}

	// ---------------------------
	// 2) Branch by method :
	//    DELETE → POST → GET → otherwise 405
	// ---------------------------

	const std::string method = req.getMethod();
	
	// ============================== DELETE ==============================
	if (method == "DELETE")
	{
		std::cout << " I AM INSIDE DELETEEEEEEEEEEEEE :D " << std::endl;
		// bool isDeleteAllowed = req.getMethod() == "DELETE" && loc->isMethodAllowed(allowedMethods, DELETE);
		if (loc && !loc->isMethodAllowed(allowedMethods, DELETE))
		{
			headers["Allow"] = "GET, POST, DELETE";
			renderError(405, "Method Not Allowed", server);
			return;
		}

		// // Check if the path is under the root
		// if (!isUnderRootAbs(absPath, absRoot))
		// {
		// 	renderError(403, "Forbidden", server);
		// 	return;
		// }
		// Check if the path is the root directory
		if (absPath == absRoot)
		{
			renderError(403, "Forbidden", server); // do not allow deleting root directory
			return;
		}
		// Only regular files can be deleted
		off_t sizeTmp = 0;
		if (!isRegular(absPath, sizeTmp))
		{
			// if is directory or if not found
			if (isDirectory(absPath))
			{
				renderError(403, "Forbidden", server); // do not allow dlt directory
				return;
			}
			renderError(404, "Not Found", server);
			return;
		}

		if (::unlink(absPath.c_str()) != 0)
		{
			switch (errno)
			{
				case EACCES:
				case EPERM:
					renderError(403, "Forbidden", server);
					return;
				case ENOENT:
					renderError(404, "Not Found", server);
					return;
				case ENOTDIR:
					renderError(404, "Not Found", server);
					return;
				case EISDIR:
					renderError(403, "Forbidden", server); // do not allow dlt directory
					return;
				case EBUSY:
				case ETXTBSY:
				case EROFS:
					renderError(409, "Conflict", server);
					return;
				default:
					renderError(500, "Internal Server Error", server);
					return;
			}
		}

		statusCode = "200 OK";
		contentType = "text/plain; charset=iso-8859-1";
		body = "Deleted\n";
		return;
	}
	// ============================== POST ==============================
	if (method == "POST")
	{	
		std::cout << " I AM INSIDE POSSSSSSSSSTTTTTTTTTTTTTT :D " << std::endl;
		if (loc && !loc->isMethodAllowed(allowedMethods, POST))
		{
			headers["Allow"] = "GET, POST, DELETE";
			renderError(405, "Method Not Allowed", server);
			return;
		}

		std::cout << "[DBG] Body temp file: " << req.getBodyFilePath() << std::endl;

		std::cout << "--------------------" << req.getBodyFilePath() << std::endl;

		std::string uploadsDir = serverRoot;

		// Ensure uploadsDir exists (optional: create if not exists)
		// if (uploadsDir.size() >= 7 && uploadsDir.substr(uploadsDir.size() - 7) != "/uploads")
		// 	uploadsDir += "/uploads";

		
		std::string filename = req.getUploadedFilename();
		
		std::string fileData;
		
		if (!filename.empty())
		{
			if (filename.find('/') != std::string::npos)
			filename = filename.substr(filename.find_last_of('/') + 1);
			fileData = req.getUploadedFileData();
		}
		else
		{
			uploadsDir = serverRoot + "/uploads";
			filename = "upload.txt";
			std::ifstream bodyFile(req.getBodyFilePath().c_str(), std::ios::in | std::ios::binary);
			if (bodyFile)
			{
				std::ostringstream ss;
				ss << bodyFile.rdbuf();
				fileData = ss.str();
			}
			else
			fileData = "";
		}
		
		struct stat st;
		if (stat(uploadsDir.c_str(), &st) != 0)
			mkdir(uploadsDir.c_str(), 0755);

		std::string uploadPath = uploadsDir + "/" + filename;

		if (fileData.empty())
		{
			renderError(400, "Empty POST body", server);
			return;
		}

		size_t maxBodySize = server->getClientMaxBodySize();
		std::cout << "----- this is maxBody size------: " << server->getClientMaxBodySize() << std::endl;
		if (req.getBodySize() > maxBodySize)
		{
			renderError(413, "Payload to large amk!", server);
			return;
		}

		if (isDirectory(uploadPath))
		{
			renderError(403, "Cannot POST to a directory", server);
			return;
		}

		// Write the POST body to the uploads folder
		std::ofstream ofs(uploadPath.c_str(), std::ios::out | std::ios::binary);
		if (!ofs)
		{
			renderError(500, "Failed to open file for POST", server);
			return;
		}
		ofs.write(fileData.c_str(), fileData.size());
		ofs.close();
	
		std::cout << "Expected body size: " << req.getBodySize() << std::endl;
		std::cout << "fileDATA size: " << fileData.size() << std::endl;
		
		// Check if file was written and size matches
		off_t writtenSize = 0;
		if (!isRegular(uploadPath, writtenSize) || static_cast<size_t>(writtenSize) != fileData.size())
		{
			renderError(500, "Failed to write POST body", server);
			return;
		}
		std::cout << "Written file size: " << writtenSize << std::endl;
	
		statusCode = "201 Created";
		contentType = "text/html; charset=iso-8859-1";
		body = "<!DOCTYPE html>\n"
			   "<html>\n"
			   "<head>\n"
			   "<title>Resource Created</title>\n"
			   "</head>\n"
			   "<body>\n"
			   "<h1>Resource created in uploads.</h1>\n"
			   "<button onclick=\"window.history.back();\">Return</button>\n"
			   "</body>\n"
			   "</html>\n";
		return;
	}

	// ========== GET ==========
	if (method == "GET")
	{
		std::cout << " I AM INSIDE GETTTTTTTTTTTTTT :D " << std::endl;
		// Respect method policy
		if (loc && !loc->isMethodAllowed(allowedMethods, GET))
		{
			headers["Allow"] = "GET, POST, DELETE";
			renderError(405, "Method Not Allowed", server);
			return;
		}

		// If directory, try index.html (or configured index). If still a dir:
		//    - autoindex off -> 403 (we'll add actual listing later)
		//    - autoindex on  -> minimal 200 placeholder for now
		if (isDirectory(absPath))
		{
			std::cout << "[DBG] index configured: " << indexName << "\n";

			if (!indexName.empty())
			{
				std::string withIndex = absPath;
				if (withIndex.size() == 0 || withIndex[withIndex.size() - 1] != '/')
				{
					withIndex += "/";
				}
				withIndex += indexName;
			
				off_t idxSz = 0;
				if (isRegular(withIndex, idxSz))
				{
					std::string data = slurpFile(withIndex);
					if (data.empty() && idxSz > 0)
					{
						renderError(500, "Internal Server Error", server);
						return;
					}
					statusCode = "200 OK";
					contentType = guessType(withIndex);
					body = data;
					return;
				}
			}

			if (!autoIndex)
			{
				renderError(403, "Forbidden", server);
				return;
			}
			// Ensure requestPath ends with '/'
			std::string requestPath = rawTarget;
			if (requestPath.empty() || requestPath[requestPath.size() - 1] != '/')
			{
				requestPath += "/";
			}
			// Build listing HTML relative to request path (not the filesystem path)
			contentType = "text/html; charset=iso-8859-1";
			statusCode = "200 OK";
			body = buildAutoindexHtml(serverRoot, absPath, requestPath);
			return;
		}

		// If regular file, serve it
		off_t size = 0;
		if (isRegular(absPath, size))
		{
			std::string data = slurpFile(absPath);
			if (data.empty() && size > 0)
			{
				renderError(500, "Internal Server Error", server);
				return;
			}
			statusCode = "200 OK";
			contentType = guessType(absPath);
			body = data;
			return;
		}
		// Not found
		renderError(404, "Not Found", server);
		return;
	}
	renderError(405, "Method Not Allowed", server);
	return;
}

// Setters
void HttpResponse::setStatusCode(const std::string &code)
{
	statusCode = code;
}

void HttpResponse::setContentType(const std::string &type)
{
	contentType = type;
}

void HttpResponse::setBody(const std::string &data)
{
	body = data;
}
void HttpResponse::addHeader(const std::string &key, const std::string &value)
{
	headers[key] = value;
}