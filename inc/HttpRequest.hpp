/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 15:19:17 by kkaratsi          #+#    #+#             */
/*   Updated: 2025/08/19 00:29:27 by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

enum class Method {GET,POST,DELETE, INVALID};
enum class Version {HTTP_1_0, HTTP_1_1, HTTP_2, INVALID};
enum class ParseState {START_LINE, HEADERS, BODY, CHUNK_SIZE, CHUNK_DATA, CHUNK_CRLF, COMPLETE, ERROR};      // CHUNK_CRLF = Chunk Carriage Return Line Feed (\r\n)
enum class ParseResult {COMPLETE, INCOMPLETE, ERROR};

// i can do a ConnectionState also CLOSED/OPEN/PENDING


class	HttpRequest
{
	private:
	// parsed data
	Method												method;
	Version												version;
	std::unordered_map<std::string, std::string> 		headers;
	std::string											path;
	std::string											rawRequest;
	
	// internal state
	ParseState											parseState;
	ParseResult											parseResult;
	size_t												content_length;
	size_t												chunk_remain_bytes;
	
	std::ofstream										bodyFile;
	size_t												bodySize;
	std::string											bodyFilePath;
		

	public:
		// OCF
		HttpRequest();
		HttpRequest(const HttpRequest &copy);
		HttpRequest &operator=(const HttpRequest &copy);
		~HttpRequest();

		// setter
		void setMethod(Method method);
		void setPath(const std::string &path);
		void setVersion(Version version);
		void setBody(const std::string &filePath);
		// void setHeaders(const std::unordered_map<std::string, std::string> &headers);

		// getter
		std::string	getMethod() const;
		std::string	getPath() const;
		std::string getVersion() const;
		size_t		getBodySize() const { return bodySize; }
		std::string	getBodyFilePath() const;
		std::unordered_map<std::string, std::string> getHeaders() const;
		
		
		
		// Parsing
		bool		parseStartLine(const std::string &line);
		bool    	parseHeadersBlock(const std::string &headerBlocks);
		ParseResult	parse();
		
		ParseResult handleChunkSize(std::string &rawRequest);
		ParseResult handleChunkData(std::string &rawRequest);
		ParseResult handleChunkCRLF(std::string &rawRequest);
		
		
		// Validation
		bool isValidMethod() const;
		bool isValidVersion() const;
		bool isValidPath();
		
		// Helper
		std::string_view	trim(std::string_view str);
		void				log_headers();
		void				log_first_line();
		void				reset();
		
		// From String to enum 
		Method		toMethodEnum(const std::string &methodStr);
		Version		toVersionEnum(const std::string &versionStr);
		
		std::string readFile(const std::string& filePath) const;
		std::string buildResponse();


		bool receiveReq(int client_fd);

		bool disconnect;
		

	};

std::ostream &operator<<(std::ostream &os, const HttpRequest &request);