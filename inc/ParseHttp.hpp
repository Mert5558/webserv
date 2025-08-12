/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseHttp.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 15:19:17 by kkaratsi          #+#    #+#             */
/*   Updated: 2025/08/11 13:46:26 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

enum class Method {GET,POST,DELETE, INVALID};
enum class Version {HTTP_1_0, HTTP_1_1, HTTP_2, INVALID};
enum class ParseState {START_LINE, HEADERS, BODY, COMPLETE, ERROR};
enum class ParseResult {COMPLETE, INCOMPLETE, ERROR};

/*
enum class ParseResult		//? later i can improve it 
{
    COMPLETE,
    INCOMPLETE,
    INVALIDSTARTLINE,
    INVALIDHEADER,
    BODYTOOLARGE,
    INTERNALERROR
};
*/

class	HttpRequest
{
	private:
		Method												method;
		std::string											path;
		Version												version;
		std::ofstream										bodyFile;
		size_t												bodySize;
		std::string											bodyFilePath;
		std::unordered_map<std::string, std::string> 		headers;
		ParseState											parseState;
		ParseResult											parseResult;
		

	public:
		HttpRequest();
		HttpRequest(const HttpRequest &copy);
		HttpRequest &operator=(const HttpRequest &copy);
		~HttpRequest();

		void setMethod(Method method);
		void setPath(const std::string &path);
		void setVersion(Version version);
		void setBody(const std::string &filePath);
		void setHeaders(const std::unordered_map<std::string, std::string> &headers);

		std::string	getMethod() const;
		std::string	getPath() const;
		std::string getVersion() const;
		size_t		getBodySize() const;
        std::string	getBodyFilePath() const;
		std::unordered_map<std::string, std::string> getHeaders() const;
		
		// From String to enum 
		Method		toMethodEnum(const std::string &methodStr);
		Version		toVersionEnum(const std::string &versionStr);
		
		// Parsing
		ParseResult	parseRequestPartial(std::string &buffer);
		bool parseRequestFromCompleteBuffer(const std::string &rawRequest);
		bool parseStartLine(const std::string &line);
		bool parseHeaders(const std::string &line);
		
		// validation
		bool isValidMethod() const;
		bool isValidVersion() const;
		bool isValidPath();
		
		std::string receiveRequest(int client_fd);
		ssize_t	receive(int client_fd, std::string &buffer);
		void log_headers();
		void log_first_line();

		// std::string buildResponse();
		std::string readFile(const std::string& filePath);
		std::string buildResponse();

		void reset();


	};

std::ostream &operator<<(std::ostream &os, const HttpRequest &request);