/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: merdal <merdal@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 15:30:19 by merdal            #+#    #+#             */
/*   Updated: 2025/07/10 15:55:27 by merdal           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Webserv.hpp"
#include "../inc/Server.hpp"
#include "../inc/ParseConfig.hpp"

int main(int argc, char **argv)
{
	if (argc == 1 || argc == 2)
	{
		ParseConfig	parse;
		std::string configName;
		std::cout << "Webserv is starting..." << std::endl;

		if (argc == 1)
			configName = "conf/default.conf";
		else
			configName = argv[1];

		parse.parseFile(configName);
		
	}
	else
	{
		std::cout << "Error: Wrong number of arguments!" << std::endl;
	}
	
}