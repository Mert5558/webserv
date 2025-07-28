/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseHTTP.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkaratsi <kkaratsi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 15:19:17 by kkaratsi          #+#    #+#             */
/*   Updated: 2025/07/28 15:55:32 by kkaratsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

class	HttpRequest
{
	private:
		std::string										method;
		std::string										path;
		std::string										version;
		std::string										body;
		std::unordered_map<std::string, std::string>	headers;

		public:
		HttpRequest();
		HttpRequest(const HttpRequest &copy);
		HttpRequest &operator=(const HttpRequest &copy);
		~HttpRequest();

		std::string	getMethod();
		std::string	getPath();
		std::string getVersion();
		std::string	getBody();
		std::unordered_map<std::string, std::string> getHeaders();
		
};