/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 13:18:46 by isromero          #+#    #+#             */
/*   Updated: 2024/08/12 14:39:09 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server() : _globalConfig("var/www/config/default.conf"), _socket(_globalConfig)
{
	try
	{
		this->_socket.init();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error initializing server: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

Server::Server(const std::string &configFilePath) : _globalConfig(configFilePath), _socket(_globalConfig)
{
	try
	{
		this->_socket.init();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error initializing server: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

Server::Server(const Server &other) : _globalConfig(other._globalConfig), _socket(other._socket) {}

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		this->_globalConfig = other._globalConfig;
		this->_socket = other._socket;
	}
	return *this;
}

Server::~Server() {}

#if defined(__linux__)
void Server::runLinux()
{
	int serverfd = this->_socket.getServerFd();

	// Create the epoll instance
	int epollfd = epoll_create1(0);
	if (epollfd == -1)
	{
		std::cerr << "Error: creating the epoll instance: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	struct epoll_event event, events[MAX_EVENTS];
	memset(&event, 0, sizeof(event));
	event.events = EPOLLIN;
	event.data.fd = serverfd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serverfd, &event) == -1)
	{
		std::cerr << "Error: adding the server socket to the epoll instance: " << strerror(errno) << std::endl;
		close(epollfd);
		exit(EXIT_FAILURE);
	}

	std::cout << "Server is running on port 6969..." << std::endl
			  << std::endl;

	// Main loop
	while (1)
	{
		int nevents = epoll_wait(epollfd, events, MAX_EVENTS, 0); // 0 means that epoll_wait will return immediately(Non-blocking)
		if (nevents == -1)
		{
			std::cerr << "Error: waiting for events: " << strerror(errno) << std::endl;
			close(epollfd);
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i < nevents; i++)
		{
			if (events[i].data.fd == serverfd)
			{
				int clientfd = this->_acceptClient();
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
			}
			else
			{
				int clientfd = events[i].data.fd;
				std::string response = this->_processRequestResponse(clientfd);
				this->_sendResponse(clientfd, response);
				close(clientfd);
			}
		}
	}
	close(epollfd);
}
#elif defined(__APPLE__)
void Server::runMac()
{
	int serverfd = this->_socket.getServerFd();

	// Create the kqueue instance
	int kqueuefd = kqueue();
	if (kqueuefd == -1)
	{
		std::cerr << "Error: creating the kqueue instance: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	struct kevent event, events[MAX_EVENTS];
	EV_SET(&event, serverfd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	if (kevent(kqueuefd, &event, 1, NULL, 0, NULL) == -1)
	{
		std::cerr << "Error: adding the server socket to the kqueue instance: " << strerror(errno) << std::endl;
		close(kqueuefd);
		exit(EXIT_FAILURE);
	}

	std::cout << "Server is running on port 6969..." << std::endl
			  << std::endl;

	// Main loop
	while (1)
	{
		int nevents = kevent(kqueuefd, NULL, 0, events, MAX_EVENTS, NULL);
		if (nevents == -1)
		{
			std::cerr << "Error: waiting for events: " << strerror(errno) << std::endl;
			close(kqueuefd);
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i < nevents; i++)
		{
			if (static_cast<int>(events[i].ident) == serverfd)
			{
				int clientfd = this->_acceptClient();

				// Add the client socket to the kqueue instance
				EV_SET(&event, clientfd, EVFILT_READ, EV_ADD, 0, 0, NULL);
				if (kevent(kqueuefd, &event, 1, NULL, 0, NULL) == -1)
				{
					std::cerr << "Error: adding the client socket to the kqueue instance: " << strerror(errno) << std::endl;
					close(clientfd);
					continue;
				}
			}
			else
			{
				int clientfd = events[i].ident;
				std::string response = this->_processRequestResponse(clientfd);
				this->_sendResponse(clientfd, response);
				close(clientfd);
			}
		}
	}
	close(kqueuefd);
}
#endif

int Server::_acceptClient()
{
	int serverfd = this->_socket.getServerFd();
	// Accept the incoming connection
	struct sockaddr_in clientAddress;
	socklen_t clientAddressSize = sizeof(clientAddress);
	int clientfd = accept(serverfd, (struct sockaddr *)&clientAddress, &clientAddressSize);
	if (clientfd == -1)
	{
		std::cerr << "Error: accepting the incoming connection: " << strerror(errno) << std::endl;
		close(clientfd);
		return -1;
	}
	return clientfd;
}

static std::pair<std::string, int> getHostInfo(int clientfd)
{
	char buffer[1024];
	ssize_t bytesRead = recv(clientfd, buffer, sizeof(buffer) - 1, MSG_PEEK); // MSG_PEEK flag reads the data without removing it from the queue
	if (bytesRead > 0)
	{
		buffer[bytesRead] = '\0';
		std::string headers(buffer);
		size_t endOfHeaders = headers.find("\r\n\r\n");
		if (endOfHeaders != std::string::npos)
		{
			headers = headers.substr(0, endOfHeaders);
			size_t hostPos = headers.find("Host: ");
			if (hostPos != std::string::npos)
			{
				size_t endOfHost = headers.find("\r\n", hostPos);
				std::string hostLine = headers.substr(hostPos + 6, endOfHost - (hostPos + 6));
				size_t colonPos = hostLine.find(':');
				if (colonPos != std::string::npos)
					return std::make_pair(hostLine.substr(0, colonPos), std::atoi(hostLine.substr(colonPos + 1).c_str()));
				else
					return std::make_pair(hostLine, 6969);
			}
		}
	}
	return std::make_pair("localhost", 6969); // Default host and port
}

std::string Server::_processRequestResponse(int clientfd)
{
	StatusCode statusCode = NO_STATUS_CODE;

	std::pair<std::string, int>
		hostInfo = getHostInfo(clientfd);											   // Get the host and port from the request
	std::pair<std::string, int> destInfo = this->_socket.getDestinationInfo(clientfd); // Get the IP and port of the destination for choose the server block because we create only one Socket for all the server blocks

	const std::pair<bool, ServerConfig> config = this->_globalConfig.getServerConfig(hostInfo, destInfo);
	if (!config.first) // No server block found for the host
	{
		statusCode = ERROR_400;
		Response response;
		return response.handleResponse(statusCode);
	}

	Request request(clientfd, config.second);
	statusCode = request.parseRequest();
	std::cout << request.getRequest() << std::endl;
	Response response(request.getRequest(), request.getMethod(), request.getPath(), request.getHeaders(), request.getBody(), config.second);
	if (statusCode != NO_STATUS_CODE)
		return response.handleResponse(statusCode);

	statusCode = response.handleMethods(); // statusCode never is going to be NO_STATUS_CODE, will be an error or success
	return response.handleResponse(statusCode);
}

void Server::_sendResponse(int clientfd, const std::string &response)
{
	if (response.empty()) // Client disconnected, so we don't need to send a response
		return;

	ssize_t bytesSent = send(clientfd, response.c_str(), response.size(), 0);
	if (bytesSent == -1)
		std::cerr << "Error: sending the response: " << strerror(errno) << std::endl;
	else
		std::cout << response << std::endl;
}
