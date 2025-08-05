/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseHttp.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkaratsi <kkaratsi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 15:19:17 by kkaratsi          #+#    #+#             */
/*   Updated: 2025/08/05 23:02:12 by kkaratsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

/* Useful link ----> https://datatracker.ietf.org/doc/html/rfc7231#section-1.1 */

enum class Method {GET,POST,DELETE, INVALID};

class	HttpRequest
{
	private:
		Method												method;
		std::string											path;
		std::string											version;
		std::string											body;
		std::vector<std::pair<std::string, std::string>>	headers;

	public:
		HttpRequest();
		HttpRequest(const HttpRequest &copy);
		HttpRequest &operator=(const HttpRequest &copy);
		~HttpRequest();

		// void setMethod(const std::string &method);
		void setMethod(Method method);
		void setPath(const std::string &path);
		void setVersion(const std::string &version);
		void setBody(const std::string &body);
		void setHeaders(const std::vector<std::pair<std::string, std::string>> &headers);

		std::string	getMethod() const;
		std::string	getPath() const;
		std::string getVersion() const;
		std::string	getBody() const;
		std::vector<std::pair<std::string, std::string>> getHeaders() const;
																			
		bool parseRequest(const std::string &rawRequest);
		bool isValidMethod() const;
		bool isValidVersion();
		bool isValidPath();
		
		std::string receiveRequest(int client_fd);
		void log_headers(const std::vector<std::pair<std::string, std::string>> &headers);
		void log_first_line();

		std::string buildResponse();
		std::string readFile(const std::string& filePath);

	};

std::ostream &operator<<(std::ostream &os, const HttpRequest &request);