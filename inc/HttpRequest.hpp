/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkaratsi <kkaratsi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 15:19:17 by kkaratsi          #+#    #+#             */
/*   Updated: 2025/08/14 16:00:25 by kkaratsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

enum class Method {GET,POST,DELETE, INVALID};
enum class Version {HTTP_1_0, HTTP_1_1, HTTP_2, INVALID};
enum class ParseState {START_LINE, HEADERS, BODY, CHUNK_SIZE, CHUNK_DATA, CHUNK_CRLF, COMPLETE, ERROR};      // CHUNK_CRLF = Chunk Carriage Return Line Feed (\r\n)
enum class ParseResult {INCOMPLETE, COMPLETE ,ERROR};


class	HttpRequest
{
	private:
		// parsed data
		Method												method;
		Version												version;
		std::map<std::string, std::string> 					headers;
		std::string											path;
		
		// internal state
		ParseState											parseState;
		ParseResult											parseResult;
		size_t												content_length;
		bool												is_chunked;
		size_t												expected_chunk_size;
		
		std::ofstream										bodyFile;
		size_t												bodySize;
		std::string											bodyFilePath;
		

	public:
		HttpRequest();
		// HttpRequest(const HttpRequest &copy);
		HttpRequest &operator=(const HttpRequest &copy);
		~HttpRequest();

		// Setter
		void setMethod(Method method);
		void setPath(const std::string &path);
		void setVersion(Version version);
		void setBody(const std::string &filePath);
		void setHeaders(const std::map<std::string, std::string> &headers);

		// Getter
		std::string	getMethod() const;
		std::string	getPath() const;
		std::string getVersion() const;
		size_t		getBodySize() const;
        std::string	getBodyFilePath() const;
		std::map<std::string, std::string> getHeaders() const;
		
		// From String to enum 
		Method		toMethodEnum(const std::string &methodStr);
		Version		toVersionEnum(const std::string &versionStr);
		
		// Parsing
		ParseResult	parse(std::string &buffer);
		
		// bool parseRequestFromCompleteBuffer(const std::string &rawRequest);
		bool parseStartLine(const std::string &line);
		// bool parseHeaders(const std::string &line);
		bool    parseHeadersBlock(const std::string &headerBlocks);
		
		// Validation
		bool isValidMethod() const;
		bool isValidVersion() const;
		bool isValidPath();
		
		// std::string receiveRequest(int client_fd);
		// ssize_t	receive(int client_fd, std::string &buffer);
		
		// Helper
		void log_headers();
		void log_first_line();
		std::string_view trim(std::string_view str);

		std::string readFile(const std::string& filePath) const;
		std::string buildResponse();

		void reset();


	};

std::ostream &operator<<(std::ostream &os, const HttpRequest &request);