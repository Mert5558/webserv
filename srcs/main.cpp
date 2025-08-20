/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkaratsi <kkaratsi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 15:30:19 by merdal            #+#    #+#             */
/*   Updated: 2025/08/20 17:16:52 by kkaratsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Webserv.hpp"
#include "../inc/ParseConfig.hpp"
#include "../inc/ServerLoop.hpp"
#include "../inc/HttpRequest.hpp"
#include "../inc/Cgi.hpp"

void	testCgi()
{
	std::map<std::string,std::string> env;
    env["REQUEST_METHOD"] = "GET";
    env["SCRIPT_FILENAME"] = "/Users/kkaratsi/Desktop/mak_branch/www/cgi-bin/script.py";
	env["CONTENT_TYPE"] = "text/html";

    Cgi cgi("/Users/kkaratsi/Desktop/mak_branch/www/cgi-bin/script.py", env);

    try
	{
		Location loc;
		loc.setCgiPath("/usr/bin/python3 /bin/bash");
        loc.setCgiExt(".py .sh");
		
		auto [status, output] = cgi.execute("", loc);
        if (status == CgiStatus::SUCCESS)
            std::cout << "CGI output:\n" << output << std::endl;
        else
            std::cerr << "CGI execution failed" << std::endl;
    }
	catch (const std::exception &e)
	{
        std::cerr << "CGI error: " << e.what() << std::endl;
    }
}

int main(int argc, char **argv)
{
	// testCgi();
	
	if (argc == 1 || argc == 2)
	{
		ParseConfig	parse;
		ServerLoop	ServerLoop;
		std::string configName;
		std::cout << "Webserv is starting..." << std::endl;

		if (argc == 1)
			configName = "conf/default.conf";
		else
			configName = argv[1];

		try
		{
			parse.parseFile(configName);
			ServerLoop.startServer(parse);
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