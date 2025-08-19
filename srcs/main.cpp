/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkaratsi <kkaratsi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 15:30:19 by merdal            #+#    #+#             */
/*   Updated: 2025/08/19 19:29:26 by kkaratsi         ###   ########.fr       */
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
    env["SCRIPT_FILENAME"] = "/Users/kkaratsi/Desktop/mak_branch/www/cgi_script.py";

    Cgi cgi("/Users/kkaratsi/Desktop/mak_branch/www/cgi_script.py", env);

    try
	{
        auto [status, output] = cgi.execute("");
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
	testCgi();
	
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