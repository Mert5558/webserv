/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRespose.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmakario <cmakario@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/01 16:51:57 by cmakario          #+#    #+#             */
/*   Updated: 2025/08/01 17:00:49by cmakario         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

class httpResponse
{
private:
	std::string statusCode; // e.g., "200 OK", "404 Not Found"
	std::string contentType; // e.g., "text/html", "application/json"
	std::string body;         // The response body content
	std::map<std::string, std::string> headers; // Additional headers

public:
	//========== OCF==========
	httpResponse();
	httpResponse(const std::string &statusCode, const std::string &contentType, const std::string &body);
	~httpResponse();

	void setStatusCode(const std::string &statusCode);
	void setContentType(const std::string &contentType);
	void setBody(const std::string &body);
	void addHeader(const std::string &key, const std::string &value);

	std::string buildResponse() const; // Builds the complete HTTP response string

	std::string generateHttpResponse(int statusCode, const std::string& contentType, const std::string& body) const;
	//========== Getters ==========
	std::string getStatusCode() const;
	std::string getContentType() const;
	std::string getBody() const;
	std::map<std::string, std::string> getHeaders() const;
}

#endif // !HTTPRESPONSE_HPP