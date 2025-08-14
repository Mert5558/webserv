#include "../inc/httpResponse.hpp"

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





std::string httpResponse::buildResponse(const HttpRequest& request) const
{
    std::string status;
    std::string content_type;
    std::string body;

    if(request.getPath() == "/")
    {
        status = "HTTP/1.1 200 OK";
        content_type = "text/html; charset=utf-8";
        body = request.readFile("./www/index2.html");
    }
    else if (request.getPath() == "/assets/images") // Fixed 'path' to 'request.getPath()'
    {
        status = "HTTP/1.1 200 OK";
        content_type = "image/svg+xml";
        body = request.readFile("./www/assets/images/logo.svg");
    }
    else
    {
        status = "HTTP/1.1 404 Not Found";
        content_type = "text/html; charset=utf-8";
        body = request.readFile("./www/Error/404.html");
    }

    //Assemble the complete HTTP response
    std::ostringstream response;
    response << status << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "\r\n";
    response << body;

    // std::cout << "---> HTTP Response:\n" << response.str() << std::endl; // print the complete HTTP response
    return response.str();
}


bool httpResponse::isDone() const
{
	return done;
}




