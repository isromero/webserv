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
#include <fcntl.h>
#include <sstream>

#define MAX_CLIENTS 10000
#define MAX_EVENTS 10000

std::string readFile(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
	{
		std::cerr << "Error: opening the file: " << strerror(errno) << std::endl;
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

std::string determineContentType(const std::string &file)
{
	if (file.find(".html") != std::string::npos)
		return "text/html";
	else if (file.find(".css") != std::string::npos)
		return "text/css";
	else if (file.find(".js") != std::string::npos)
		return "application/javascript";
	else if (file.find(".jpg") != std::string::npos)
		return "image/jpeg";
	else if (file.find(".png") != std::string::npos)
		return "image/png";
	else if (file.find(".gif") != std::string::npos)
		return "image/gif";
	else if (file.find(".ico") != std::string::npos)
		return "image/x-icon";
	else
		return "text/plain";
}

std::string handleGetRequest(const std::string &requestedFile)
{
	std::string response;
	std::string file;
	std::string status;

	if (requestedFile == "/")
	{
		file = "pages/index.html";
		status = "200 OK";
	}
	else
	{
		file = "pages" + requestedFile;

		if (file.find(".html") == std::string::npos)
			file += ".html";
		if (open(file.c_str(), O_RDONLY) == -1)
		{
			status = "404 Not Found";
			file = "pages/404.html";
		}
		else
			status = "200 OK";
	}
	std::string fileContent = readFile(file);
	std::ostringstream ss;
	ss << fileContent.size();
	std::string contentType = determineContentType(file);
	response = "HTTP/1.1 " + status + "\r\n" +
			    "Content-Type: " +
			   contentType + "\r\n" +
			   "Content-Length: " +
			   ss.str() + "\r\n\r\n" + fileContent;

	return response;
}

std::string handleMethod(const std::string &request)
{
	std::string response;
	std::string method;
	std::string requestedFile;

	size_t methodEnd = request.find(" ");
	if (methodEnd != std::string::npos)
	{
		method = request.substr(0, methodEnd);
		size_t fileStart = request.find("/", methodEnd);
		size_t fileEnd = request.find(" ", fileStart + 1);
		if (fileStart != std::string::npos && fileEnd != std::string::npos)
			requestedFile = request.substr(fileStart, fileEnd - fileStart);
	}

	if (method == "GET")
		response = handleGetRequest(requestedFile);
	// else if (method == "POST")
	// {
	// 	response = handlePostRequest();
	// }
	// else if (method == "DELETE")
	// {
	// 	response = handleDeleteRequest();
	// }
	// else
	// {
	// 	response = "HTTP/1.0 405 Method Not Allowed\r\n"
	// 			   "Content-Type: text/plain\r\n"
	// 			   "Content-Length: 0\r\n\r\n";
	// }

	return response;
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
	memset(&event, 0, sizeof(event));
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
				memset(&event, 0, sizeof(event));
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

				std::string response = handleMethod(request);

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
