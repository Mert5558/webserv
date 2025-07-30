/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkaratsi <kkaratsi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 15:30:19 by merdal            #+#    #+#             */
/*   Updated: 2025/07/30 14:08:14 by kkaratsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Webserv.hpp"
#include "../inc/ParseConfig.hpp"
#include "../inc/ParseHttp.hpp"


int main(int argc, char **argv)
{
	if (argc == 1 || argc == 2)
	{
		HttpRequest	request;
		ParseConfig	parse;
		std::string configName;
		std::cout << "Webserv is starting..." << std::endl;

		if (argc == 1)
			configName = "conf/default.conf";
		else
			configName = argv[1];
		try
		{
			parse.parseFile(configName);
			// networking()
			
			// 1. Create socket
			int server_fd = socket(AF_INET, SOCK_STREAM, 0);
			if (server_fd < 0)
			{
				perror("socket");
				return 1;
			}

			// 2. Allow address reuse
			int opt = 1;
			setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

			// 3. Bind socket to IP/port
			struct sockaddr_in server_addr;
			std::memset(&server_addr, 0, sizeof(server_addr));
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(PORT);
			server_addr.sin_addr.s_addr = INADDR_ANY;

			if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
			{
				perror("bind");
				close(server_fd);
				return 1;
			}

			// 4. Listen for incoming connections
			if (listen(server_fd, BACKLOG) < 0)
			{
				perror("listen");
				close(server_fd);
				return 1;
			}

			std::cout << "Server listening on port " << PORT << "..." << std::endl;

			// 5. Set up poll
			struct pollfd fds[1];
			fds[0].fd = server_fd;
			fds[0].events = POLLIN;

			while (true)
			{
				int ret = poll(fds, 1, -1);
				if (ret < 0)
				{
					perror("poll");
					break;
				}

				if (fds[0].revents & POLLIN)
				{
					// 6. Accept a new connection
					struct sockaddr_in client_addr;
					socklen_t client_len = sizeof(client_addr);
					int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
					if (client_fd < 0)
					{
						perror("accept");
						continue;
					}

					std::cout << "\n" << "Client connected: " << inet_ntoa(client_addr.sin_addr) << std::endl;

					// Handle multiple requests from the same client
					while (true)
					{
						std::string buffer = request.receiveRequest(client_fd);
						request.parseRequest(buffer);
						// request.isValidMethod(); // maybe we should encaptulate the parseRequest() function iside this one and rename it
						
						
						// 8. Send a basic HTTP response with keep-alive
						std::string response = request.buildResponse();
						
						int bytes_sent = send(client_fd, response.c_str(), response.size(), 0);
						if (bytes_sent < 0)
						{
							perror("send");
							break;
						}

						std::cout << "Response sent (" << bytes_sent << " bytes)." << std::endl;
					}
					close(client_fd); // Close the connection when the client disconnects
				}
				close(server_fd);
			}
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