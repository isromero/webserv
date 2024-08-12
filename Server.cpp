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

Server::Server() : _port(6969), _serverfd(-1)
{
	this->_createSocket();
	this->_bindSocket();
	this->_listenSocket();
}

Server::Server(int port) : _port(port), _serverfd(-1)
{
	this->_createSocket();
	this->_bindSocket();
	this->_listenSocket();
}

Server::Server(const std::string &configFile) : _port(6969), _serverfd(-1)
{
	// TODO: Do it properly
	(void)configFile;
}

Server::Server(const Server &other) : _port(other._port), _serverfd(other._serverfd)
{
}

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		this->_port = other._port;
		this->_serverfd = other._serverfd;
	}
	return *this;
}

Server::~Server()
{
}

#if defined(__linux__)
void Server::runLinux()
{
	// Create the epoll instance
	int epollfd = epoll_create1(0);
	if (epollfd == -1)
	{
		std::cerr << "Error: creating the epoll instance: " << strerror(errno) << std::endl;
		close(this->_serverfd);
		exit(EXIT_FAILURE);
	}

	struct epoll_event event, events[MAX_EVENTS];
	memset(&event, 0, sizeof(event));
	event.events = EPOLLIN;
	event.data.fd = this->_serverfd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, this->_serverfd, &event) == -1)
	{
		std::cerr << "Error: adding the server socket to the epoll instance: " << strerror(errno) << std::endl;
		close(this->_serverfd);
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
			close(this->_serverfd);
			close(epollfd);
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i < nevents; i++)
		{
			if (events[i].data.fd == this->_serverfd)
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
				std::string response = this->_processRequest(clientfd);
				this->_sendResponse(clientfd, response);
				close(clientfd);
			}
		}
	}
	close(this->_serverfd);
	close(epollfd);
}
#elif defined(__APPLE__)
void Server::runMac()
{
	// Create the kqueue instance
	int kqueuefd = kqueue();
	if (kqueuefd == -1)
	{
		std::cerr << "Error: creating the kqueue instance: " << strerror(errno) << std::endl;
		close(this->_serverfd);
		exit(EXIT_FAILURE);
	}

	struct kevent event, events[MAX_EVENTS];
	EV_SET(&event, this->_serverfd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	if (kevent(kqueuefd, &event, 1, NULL, 0, NULL) == -1)
	{
		std::cerr << "Error: adding the server socket to the kqueue instance: " << strerror(errno) << std::endl;
		close(this->_serverfd);
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
			close(this->_serverfd);
			close(kqueuefd);
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i < nevents; i++)
		{
			if (static_cast<int>(events[i].ident) == this->_serverfd)
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
				std::string response = this->_processRequest(clientfd);
				this->_sendResponse(clientfd, response);
				close(clientfd);
			}
		}
	}
	close(this->_serverfd);
	close(kqueuefd);
}
#endif

void Server::_createSocket()
{
	// AF_INET: ipv4
	// SOCK_STREAM: TCP
	// 0: Default (0 = TCP)
	this->_serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_serverfd < 0)
	{
		std::cerr << "Error: creating the socket: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
}

void Server::_bindSocket()
{
	// Create the server address
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;	 // TODO: Think if we want to do the server with Ipv4 and Ipv6 (we need to use addrinfo, gai_strerror...)
	serverAddress.sin_port = htons(this->_port); // htons converts the port number to network byte order

	// Bind the socket to the address and port
	if (bind(this->_serverfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
	{
		std::cerr << "Error: binding the socket to the address and port: " << strerror(errno) << std::endl;
		close(this->_serverfd);
		exit(EXIT_FAILURE);
	}
}

void Server::_listenSocket()
{
	// Listen for incoming connections
	if (listen(this->_serverfd, MAX_CLIENTS) == -1)
	{
		std::cerr << "Error: listening for incoming connections: " << strerror(errno) << std::endl;
		close(this->_serverfd);
		exit(EXIT_FAILURE);
	}
}

int Server::_acceptClient()
{
	// Accept the incoming connection
	struct sockaddr_in clientAddress;
	socklen_t clientAddressSize = sizeof(clientAddress);
	int clientfd = accept(this->_serverfd, (struct sockaddr *)&clientAddress, &clientAddressSize);
	if (clientfd == -1)
	{
		std::cerr << "Error: accepting the incoming connection: " << strerror(errno) << std::endl;
		close(clientfd);
		return -1;
	}
	return clientfd;
}

std::string Server::_readRequest(int clientfd)
{
	// Just read the data from the client
	char buffer[1024];
	ssize_t bytesRead;
	std::string request;
	while ((bytesRead = recv(clientfd, buffer, sizeof(buffer) - 1, 0)) > 0)
	{
		buffer[bytesRead] = '\0';
		request.append(buffer, bytesRead);
		if (request.find("\r\n\r\n") != std::string::npos)
			break;
	}
	if (bytesRead == -1)
	{
		std::cerr << "Error: reading the data from the client: " << strerror(errno) << std::endl;
		close(clientfd);
	}
	else if (bytesRead == 0)
	{
		std::cout << "Client disconnected" << std::endl;
		close(clientfd);
		return "";
	}

	return request;
}

void Server::_parseRequest(const std::string &request, std::string &method, std::string &requestedFile, std::map<std::string, std::string> &headers, std::string &body)
{
	size_t methodEnd = request.find(" ");
	if (methodEnd != std::string::npos)
	{
		method = request.substr(0, methodEnd);
		size_t fileStart = request.find("/", methodEnd);
		size_t fileEnd = request.find(" ", fileStart + 1);
		if (fileStart != std::string::npos && fileEnd != std::string::npos)
			requestedFile = request.substr(fileStart, fileEnd - fileStart);
	}

	size_t headerEnd = request.find("\r\n\r\n");
	if (headerEnd != std::string::npos)
	{
		size_t pos = request.find("\r\n") + 2;
		while (pos < headerEnd)
		{
			size_t nextPos = request.find("\r\n", pos);
			std::string headerLine = request.substr(pos, nextPos - pos);
			pos = nextPos + 2;

			size_t separator = headerLine.find(": ");
			if (separator != std::string::npos)
			{
				std::string key = headerLine.substr(0, separator);
				std::string value = headerLine.substr(separator + 2);
				headers[key] = value;
			}
		}

		// Extract the body
		body = request.substr(headerEnd + 4);
	}
}

std::string Server::_processResponse(const std::string &method, const std::string &requestedFile)
{
	std::string response = this->_handleMethods(method, requestedFile);
	return response;
}

std::string Server::_processRequest(int clientfd)
{
	std::string request = _readRequest(clientfd);
	std::cout << request << std::endl;
	if (request.empty())
		return "HTTP/1.1 400 Bad Request\r\n\r\n"; // TODO: Manage and send a proper response

	std::string method, requestedFile, body;
	std::map<std::string, std::string> headers;
	this->_parseRequest(request, method, requestedFile, headers, body);

	std::string response = this->_processResponse(method, requestedFile);
	std::cout << response << std::endl;
	return response;
}

std::string Server::_handleMethods(const std::string &method, const std::string &requestedFile)
{
	std::string response;

	if (method == "GET")
		response = Server::_handleGET(requestedFile);
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

std::string Server::_readFile(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
	{
		std::cerr << "Error: opening the file: " << strerror(errno) << std::endl;
		return "";
	}
	std::ostringstream ss;
	ss << file.rdbuf();
	file.close();
	return ss.str();
}

std::string Server::_determineContentType(const std::string &filename)
{
	if (filename.find(".html") != std::string::npos)
		return "text/html";
	else if (filename.find(".css") != std::string::npos)
		return "text/css";
	else if (filename.find(".js") != std::string::npos)
		return "application/javascript";
	else if (filename.find(".jpg") != std::string::npos)
		return "image/jpeg";
	else if (filename.find(".png") != std::string::npos)
		return "image/png";
	else if (filename.find(".gif") != std::string::npos)
		return "image/gif";
	else if (filename.find(".ico") != std::string::npos)
		return "image/x-icon";
	else
		return "text/plain";
}

std::string Server::_handleGET(const std::string &requestedFile)
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
	std::string fileContent = Server::_readFile(file);
	std::ostringstream ss;
	ss << fileContent.size();
	std::string contentType = Server::_determineContentType(file);
	response = "HTTP/1.1 " + status + "\r\n" +
			   "Content-Type: " +
			   contentType + "\r\n" +
			   "Content-Length: " +
			   ss.str() + "\r\n\r\n" + fileContent;

	return response;
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
