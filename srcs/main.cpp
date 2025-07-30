/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkaratsi <kkaratsi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 15:30:19 by merdal            #+#    #+#             */
/*   Updated: 2025/07/30 14:32:54 by kkaratsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Webserv.hpp"
#include "../inc/ParseConfig.hpp"
#include "../inc/Server.hpp"
#include "../inc/ParseHttp.hpp"


int main(int argc, char **argv)
{
	if (argc == 1 || argc == 2)
	{
		ParseConfig	parse;
		Server		server;
		std::string configName;
		std::cout << "Webserv is starting..." << std::endl;

		if (argc == 1)
			configName = "conf/default.conf";
		else
			configName = argv[1];

		try
		{
			parse.parseFile(configName);
			server.startServer(parse);
		}
		catch (const ConfigError &e)
		{
			std::cerr << e.what() << std::endl;
			return (1);
		}
	}
	else
	{
		std::cout << "Error: Wrong number of arguments!" << std::endl;
	}
}