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

Server::Server() : _config("var/www/config/default.conf"), _socket(_config)
{
	try
	{
		int port = this->_config.getPort();
		std::cout << "Port: " << port << std::endl;

		std::vector<std::string> serverNames = this->_config.getServerNames();
		for (std::vector<std::string>::const_iterator it = serverNames.begin(); it != serverNames.end(); ++it)
			std::cout << "Server name: " << *it << std::endl;

		std::string host = this->_config.getHost();
		std::cout << "Host: " << host << std::endl;

		std::string root = this->_config.getRoot();
		std::cout << "Root: " << root << std::endl;

		std::vector<std::string> indexes = this->_config.getIndexes();
		for (std::vector<std::string>::const_iterator it = indexes.begin(); it != indexes.end(); ++it)
			std::cout << "Index: " << *it << std::endl;

		size_t clientMaxBodySize = this->_config.getClientMaxBodySize();
		std::cout << "Client max body size: " << clientMaxBodySize << std::endl;

		std::map<int, std::string> errorPages = this->_config.getErrorPages();
		for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
			std::cout << "Error page: " << it->first << " " << it->second << std::endl;

		std::vector<LocationConfig> locations = this->_config.getLocations();
		for (std::vector<LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it)
		{
			std::cout << "Location path: " << it->path << std::endl;
			std::vector<std::string> allowedMethods = it->allowedMethods;
			for (std::vector<std::string>::const_iterator it2 = allowedMethods.begin(); it2 != allowedMethods.end(); ++it2)
				std::cout << "Allowed method: " << *it2 << std::endl;
			std::cout << "Autoindex: " << it->autoindex << std::endl;
			std::cout << "Upload dir: " << it->uploadDir << std::endl;
			std::cout << "CGI extension: " << it->cgiExtension << std::endl;
			std::cout << "CGI bin: " << it->cgiBin << std::endl;
			std::cout << "Redirect: " << it->redirect << std::endl;
		}
		this->_socket.init();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error initializing server: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

Server::Server(const std::string &configFilePath) : _config(configFilePath), _socket(_config)
{
	try
	{
		std::vector<std::string> serverNames = this->_config.getServerNames();
		for (std::vector<std::string>::const_iterator it = serverNames.begin(); it != serverNames.end(); ++it)
			std::cout << "Server name: " << *it << std::endl;
		this->_socket.init();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error initializing server: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

Server::Server(const Server &other) : _config(other._config), _socket(other._socket) {}

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		this->_config = other._config;
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

std::string Server::_processRequestResponse(int clientfd)
{
	Request request(clientfd, this->_config);

	StatusCode statusCode = request.parseRequest();
	std::cout << request.getRequest() << std::endl;
	Response response(request.getRequest(), request.getMethod(), request.getPath(), request.getHeaders(), request.getBody(), this->_config);
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
