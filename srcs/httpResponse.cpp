#include "../inc/HttpResponse.hpp"
#include <algorithm>
#include <cctype>
#include <cerrno>

// ========== Constructor / Destructor ==========

HttpResponse::HttpResponse()
: statusCode("200 OK"), contentType("text/plain"), body("")
{
}

HttpResponse::~HttpResponse()
{
}

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
	// Minimal and readable. Extend later as needed.
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
		return "text/html; charset=iso-8859-1";
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

// --- Path safety ---
// If you already have your own normalizePath/safeJoin, prefer using it.
// Here we implement a simple, readable fallback that still prevents ".." traversal
// per project guidance and Beej’s advice. 
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
		return path; // good enough for our purposes here
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

bool HttpResponse::isUnderRootAbs(const std::string &absPath, const std::string &absRoot)
{
	if (absRoot.empty())
	{
		return false;
	}
	std::string root = absRoot;
	if (root.size() > 1 && root[root.size() - 1] == '/')
	{
		root.erase(root.size() - 1);
	}
	if (absPath.size() < root.size())
	{
		return false;
	}
	if (absPath.compare(0, root.size(), root) != 0)
	{
		return false;
	}
	if (absPath.size() == root.size())
	{
		return true;
	}
	return absPath[root.size()] == '/';
}

// ========== Public API ==========

std::string HttpResponse::buildResponse() const
{
	std::ostringstream ss;
	ss << "HTTP/1.1 " << statusCode << "\r\n";

	// Base headers (avoid duplicating if user added them manually)
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

// *** DO NOT CHANGE THIS SIGNATURE ***
void HttpResponse::prepare(const HttpRequest &req, const InitConfig *server)
{
	// Reset minimal defaults for each response
	statusCode = "200 OK";
	contentType = "text/plain";
	body.clear();
	headers.clear();

	// 1) Methods: we fully implement GET here.
	// Others will be 405 for now (spec requires GET, POST, DELETE later). 
	if (req.getMethod() != "GET")
	{
		statusCode = "405 Method Not Allowed";
		contentType = "text/html; charset=iso-8859-1";
		body = defaultErrorBody(405, "Method Not Allowed");
		headers["Allow"] = "GET, POST, DELETE";
		return;
	}

	// 2) Server root / index / autoindex from config
	const std::string serverRoot = (server ? server->getRoot() : ".");
	const std::string indexName = (server ? server->getIndex() : "index.html");
	const bool autoIndex = (server ? server->getAutoIndex() : false);

	// 3) Resolve target path safely (Beej: guard against ".." traversal). 
	const std::string target = req.getPath().empty() ? "/" : req.getPath();
	std::string full = joinUnderRoot(serverRoot, target);

	const std::string absRoot = makeAbsolute(serverRoot);
	const std::string absPath = makeAbsolute(full);

	if (!isUnderRootAbs(absPath, absRoot))
	{
		statusCode = "403 Forbidden";
		contentType = "text/html; charset=iso-8859-1";
		body = defaultErrorBody(403, "Forbidden");
		return;
	}

	// 4) If directory, try index.html (or configured index). If still a dir:
	//    - autoindex off -> 403 (we'll add actual listing later)
	//    - autoindex on  -> minimal 200 placeholder for now
	if (isDirectory(absPath))
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
				statusCode = "500 Internal Server Error";
				contentType = "text/html; charset=iso-8859-1";
				body = defaultErrorBody(500, "Internal Server Error");
				return;
			}
			statusCode = "200 OK";
			contentType = guessType(withIndex);
			body = data;
			return;
		}

		if (!autoIndex)
		{
			statusCode = "403 Forbidden";
			contentType = "text/html; charset=iso-8859-1";
			body = defaultErrorBody(403, "Forbidden");
			return;
		}
		else
		{
			// Minimal placeholder until we implement proper listing
			statusCode = "200 OK";
			contentType = "text/plain; charset=iso-8859-1";
			body = "Autoindex enabled (listing not yet implemented)\n";
			return;
		}
	}

	// 5) If regular file, serve it
	off_t size = 0;
	if (isRegular(absPath, size))
	{
		std::string data = slurpFile(absPath);
		if (data.empty() && size > 0)
		{
			statusCode = "500 Internal Server Error";
			contentType = "text/html; charset=iso-8859-1";
			body = defaultErrorBody(500, "Internal Server Error");
			return;
		}
		statusCode = "200 OK";
		contentType = guessType(absPath);
		body = data;
		return;
	}

	// 6) Not found
	statusCode = "404 Not Found";
	contentType = "text/html; charset=iso-8859-1";
	body = defaultErrorBody(404, "Not Found");
}
