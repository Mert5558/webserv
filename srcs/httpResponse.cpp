#include "../inc/httpResponse.hpp"

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




