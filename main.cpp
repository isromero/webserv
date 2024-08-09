/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/09 16:31:34 by isromero          #+#    #+#             */
/*   Updated: 2024/08/09 16:31:34 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <sys/epoll.h>
#include <cstring>

#define MAX_CLIENTS 10000
#define MAX_EVENTS 10000

std::string readFile(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	if (!file)
	{
		std::cerr << "Error: opening the file: " << filename << ": " << strerror(errno) << std::endl;
		return "";
	}
	return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

int main()
{
	// AF_INET: ipv4
	// SOCK_STREAM: TCP
	// 0: Default (0 = TCP)
	int serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverfd == -1)
	{
		std::cerr << "Error: creating the socket: " << strerror(errno) << std::endl;
		return (1);
	}

	// Create the server address
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY; // TODO: Think if we want to do the server with Ipv4 and Ipv6 (we need to use addrinfo, gai_strerror...)
	serverAddress.sin_port = htons(6969);		// htons converts the port number to network byte order

	// Bind the socket to the address and port
	if (bind(serverfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
	{
		std::cerr << "Error: binding the socket to the address and port: " << strerror(errno) << std::endl;
		close(serverfd);
		return (1);
	}

	// Listen for incoming connections
	if (listen(serverfd, MAX_CLIENTS) == -1)
	{
		std::cerr << "Error: listening for incoming connections: " << strerror(errno) << std::endl;
		close(serverfd);
		return (1);
	}

	// Create the epoll instance
	int epollfd = epoll_create1(0);
	if (epollfd == -1)
	{
		std::cerr << "Error: creating the epoll instance: " << strerror(errno) << std::endl;
		close(serverfd);
		return (1);
	}

	// Add the server socket to the epoll instance
	struct epoll_event event, events[MAX_EVENTS];
	event.events = EPOLLIN;
	event.data.fd = serverfd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serverfd, &event) == -1)
	{
		std::cerr << "Error: adding the server socket to the epoll instance: " << strerror(errno) << std::endl;
		close(serverfd);
		close(epollfd);
		return (1);
	}

	std::cout << "Server is running on port 6969" << std::endl;

	while (1)
	{
		// Wait for events
		int numEvents = epoll_wait(epollfd, events, MAX_EVENTS, 0); // 0 means that epoll_wait will return immediately (non-blocking, mandatory in the subject)
		if (numEvents == -1)
		{
			std::cerr << "Error: waiting for events: " << strerror(errno) << std::endl;
			break;
		}

		for (int i = 0; i < numEvents; ++i)
		{
			if (events[i].data.fd == serverfd)
			{
				// Accept the incoming connection
				struct sockaddr_in clientAddress;
				socklen_t clientAddressSize = sizeof(clientAddress);
				int clientfd = accept(serverfd, (struct sockaddr *)&clientAddress, &clientAddressSize);
				if (clientfd == -1)
				{
					std::cerr << "Error: accepting the incoming connection: " << strerror(errno) << std::endl;
					continue;
				}

				// Add the client socket to the epoll instance
				event.events = EPOLLIN;
				event.data.fd = clientfd;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &event) == -1)
				{
					std::cerr << "Error: adding the client socket to the epoll instance: " << strerror(errno) << std::endl;
					close(clientfd);
					continue;
				}

				std::cout << "Accepted connection" << std::endl;
			}
			else
			{
				int clientfd = events[i].data.fd;
				char buffer[1024];
				ssize_t bytesRead;
				std::string request;

				while ((bytesRead = recv(clientfd, buffer, sizeof(buffer), 0)) > 0)
				{
					request.append(buffer, bytesRead);
					if (request.find("\r\n\r\n") != std::string::npos)
						break;
				}

				if (bytesRead == 0)
				{
					std::cout << "Client disconnected" << std::endl;
					close(clientfd);
					continue;
				}
				else if (bytesRead == -1)
				{
					std::cerr << "Error: reading from client socket: " << strerror(errno) << std::endl;
					close(clientfd);
					continue;
				}

				std::cout << "Request:" << std::endl;
				std::cout << request << std::endl;

				std::string response;
				std::string requestedFile;
				size_t pos = request.find("GET /");
				if (pos != std::string::npos)
				{
					pos += 5;
					size_t endPos = request.find(" ", pos);
					requestedFile = request.substr(pos, endPos - pos);
				}

				if (requestedFile.empty() || requestedFile == "/")
				{
					response = "HTTP/1.0 200 OK\r\n";
					response += "Content-Type: text/html\r\n";
					response += "Content-Length: 13\r\n";
					response += "\r\n";
					response += "Hello, World!";
				}
				else if (requestedFile == "index.html")
				{
					std::string htmlContent = readFile("index.html");
					if (htmlContent.empty())
					{
						response = "HTTP/1.0 500 Internal Server Error\r\n"
								   "Content-Type: text/plain\r\n"
								   "Content-Length: 0\r\n\r\n";
					}
					else
					{
						std::ostringstream contentLength;
						contentLength << htmlContent.size();

						response = "HTTP/1.0 200 OK\r\n";
						response += "Content-Type: text/html\r\n";
						response += "Content-Length: " + contentLength.str() + "\r\n";
						response += "\r\n";
						response += htmlContent;
					}
				}
				else
				{
					std::string errorContent = readFile("404.html");
					if (errorContent.empty())
					{
						response = "HTTP/1.0 404 Not Found\r\n"
								   "Content-Type: text/plain\r\n"
								   "Content-Length: 0\r\n\r\n";
					}
					else
					{
						std::ostringstream contentLength;
						contentLength << errorContent.size();

						response = "HTTP/1.0 500 Internal Server Error\r\n";
						response += "Content-Type: text/html\r\n";
						response += "Content-Length: " + contentLength.str() + "\r\n";
						response += "\r\n";
						response += errorContent;
					}
				}

				ssize_t bytesSent = send(clientfd, response.c_str(), response.size(), 0);
				if (bytesSent == -1)
					std::cerr << "Error: sending the response: " << strerror(errno) << std::endl;
				else
				{
					std::cout << "Response:" << std::endl;
					std::cout << response << std::endl;
				}
				close(clientfd);
			}
		}
	}
	close(serverfd);
	close(epollfd);
	return (0);
}
