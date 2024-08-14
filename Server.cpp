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

std::string Server::_secureFilePath(const std::string &path)
{
	std::string securePath = path;
	size_t pos = 0;
	while ((pos = securePath.find("..")) != std::string::npos)
		securePath.erase(pos, 2);
	while ((pos = securePath.find("//")) != std::string::npos)
		securePath.erase(pos, 1);
	while ((pos = securePath.find("~")) != std::string::npos)
		securePath.erase(pos, 1);
	while ((pos = securePath.find("/./")) != std::string::npos)
		securePath.erase(pos, 2);
	while ((pos = securePath.find("/../")) != std::string::npos)
		securePath.erase(pos, 3);
	return securePath;
}

// Parsing Request following RFC 9112
ParseRequestError Server::_parseRequest(const std::string &request, std::string &method, std::string &requestedFile, std::map<std::string, std::string> &headers, std::string &body)
{
	if (request.empty())
		return INVALID_REQUEST;
	else if (request.size() > 8192) // ! Maximum request size:  we can change this value
		return PAYLOAD_TOO_LARGE;

	size_t pos = 0;
	size_t end = 0;

	// Leading empty lines prior to the request-line
	while (pos < request.size() && (request[pos] == '\r' || request[pos] == '\n'))
	{
		if (request[pos] == '\r' && pos + 1 < request.size() && request[pos + 1] == '\n')
			pos += 2;
		else
			pos++;
	}

	// Parse the request-line
	end = request.find("\r\n", pos);
	if (end == std::string::npos)
	{
		end = request.find('\n', pos);
		if (end == std::string::npos)
			return INVALID_REQUEST;
	}

	std::string requestLine = request.substr(pos, end - pos);
	size_t methodEnd = requestLine.find(' ');
	size_t fileStart = requestLine.find('/', methodEnd);
	size_t fileEnd = requestLine.find(' ', fileStart + 1);
	size_t versionStart = requestLine.find("HTTP/", fileEnd);

	if (methodEnd != std::string::npos && fileStart != std::string::npos && fileEnd != std::string::npos)
	{
		method = requestLine.substr(0, methodEnd);
		if (method != "GET" && method != "POST" && method != "DELETE") // TODO: Change if we add more methods
			return INVALID_METHOD;

		std::string version = requestLine.substr(versionStart);
		if (version != "HTTP/1.1")
			return VERSION_NOT_SUPPORTED;

		std::string uri = requestLine.substr(fileStart, fileEnd - fileStart);
		if (uri.size() > 2048) // ! Maximum URI size: we can change this value
			return URI_TOO_LONG;
		if (uri.find("://") != std::string::npos)
		{
			// Absolute-form request, extract path after the authority. Eg: GET http://localhost:6969/index.html HTTP/1.1
			size_t pathStart = uri.find('/', uri.find("://") + 3);
			if (pathStart != std::string::npos)
				requestedFile = uri.substr(pathStart);
			else
				requestedFile = "/";
		}
		else
			requestedFile = uri; // Relative-form request. Eg: GET /index.html HTTP/1.1
		if (requestedFile.find(' ') != std::string::npos || requestedFile.empty())
			return INVALID_REQUEST_TARGET;
		requestedFile = this->_secureFilePath(requestedFile);
	}
	else if (requestLine.find("HTTP/1.1") == std::string::npos)
		return VERSION_NOT_SUPPORTED;
	else
		return INVALID_REQUEST_LINE;

	pos = end + 1;
	if (request[pos] == '\n')
		pos++; // Skip the \n character if we encountered \r\n

	// Check for whitespace between request-line and first header field
	if (pos < request.size() && (request[pos] == ' ' || request[pos] == '\t'))
	{
		// Whitespace detected, consume whitespace-preceded lines
		while (pos < request.size())
		{
			end = request.find("\r\n", pos);
			if (end == std::string::npos)
			{
				end = request.find('\n', pos);
				if (end == std::string::npos)
					break; // End of request
			}

			// Check if this line starts with a valid header field
			if (request[pos] != ' ' && request[pos] != '\t')
			{
				size_t colon = request.find(':', pos);
				if (colon != std::string::npos && colon < end && colon > pos)
					break; // Valid header field found, stop consuming
			}

			// Consume this line
			pos = end + 1;
			if (request[pos] == '\n')
				pos++;
		}
	}

	// Parse headers
	while (pos < request.size())
	{
		end = request.find("\r\n", pos);
		if (end == std::string::npos)
		{
			end = request.find('\n', pos);
			if (end == std::string::npos)
				break; // End of request
		}

		if (end == pos || (end == pos + 1 && request[pos] == '\r'))
		{
			// Empty line, end of headers
			pos = end + 1;
			if (request[pos] == '\n')
				pos++;
			break;
		}

		std::string headerLine = request.substr(pos, end - pos);
		size_t separator = headerLine.find(": ");
		if (separator != std::string::npos)
		{
			std::string headerName = headerLine.substr(0, separator);
			std::string headerValue = headerLine.substr(separator + 2);

			// Replace bare CR with SP in header value
			for (size_t i = 0; i < headerValue.size(); ++i)
			{
				if (headerValue[i] == '\r' && (i + 1 == headerValue.size() || headerValue[i + 1] != '\n'))
					headerValue[i] = ' ';
			}

			headers[headerName] = headerValue;
		}
		else
			return INVALID_HEADER_FORMAT;

		pos = end + 1;
		if (request[pos] == '\n')
			pos++;
	}

	// Check for Host header is only one(it is necessary just one)
	size_t hostCount = headers.count("Host");
	if (hostCount != 1)
		return INVALID_HEADER_FORMAT;

	// Check for Host header value
	std::map<std::string, std::string>::iterator hostIt = headers.find("Host");
	std::string hostValue = hostIt->second;
	std::ostringstream oss;
	oss << "localhost:" << this->_port; // TODO: Change host when we have the config file
	std::string hostWithPort = oss.str();
	if (hostValue.empty() || (hostValue != hostWithPort && hostValue != "localhost")) // TODO: Change hosts when we have the config file
		return INVALID_HEADER_FORMAT;

	// Check for Content-Length
	size_t contentLength = 0;
	std::map<std::string, std::string>::iterator it = headers.find("Content-Length");
	if (it != headers.end())
		contentLength = static_cast<size_t>(std::atoi(it->second.c_str()));
	else
	{
		// No Content-Length specified, check for Transfer-Encoding
		it = headers.find("Transfer-Encoding");
		if (it != headers.end() && it->second == "chunked") // Actually, we don't support chunked encoding
			return INVALID_CONTENT_LENGTH;
	}

	size_t remainingLength = request.size() - pos;
	if (contentLength > 0)
	{
		if (remainingLength < contentLength)
			return INCOMPLETE_BODY;
		body = request.substr(pos, contentLength);
	}
	else if (remainingLength > 0)
		body = request.substr(pos); // No Content-Length specified, consume the remaining data

	// Replace bare CR with SP in body
	for (size_t i = 0; i < body.size(); ++i)
	{
		if (body[i] == '\r' && (i + 1 == body.size() || body[i + 1] != '\n'))
			body[i] = ' ';
	}

	return NO_ERROR;
}

std::string Server::_processResponse(const std::string &method, const std::string &requestedFile)
{
	std::string response = this->_handleMethods(method, requestedFile);
	return response;
}

std::string Server::_generateErrorResponse(ParseRequestError error)
{
	std::string status_line, body;

	switch (error)
	{
	case INVALID_REQUEST:
	case INVALID_REQUEST_LINE:
	case INVALID_HEADER_FORMAT:
	case INCOMPLETE_BODY:
	case INVALID_REQUEST_TARGET:
		status_line = "HTTP/1.1 400 Bad Request";
		body = "400 Bad Request: The server cannot process the request due to a client error.";
		break;
	case INVALID_METHOD:
		status_line = "HTTP/1.1 405 Method Not Allowed";
		body = "405 Method Not Allowed: The method specified in the request is not allowed.";
		break;
	case INVALID_CONTENT_LENGTH:
		status_line = "HTTP/1.1 411 Length Required";
		body = "411 Length Required: The request did not specify the length of its content.";
		break;
	case PAYLOAD_TOO_LARGE:
		status_line = "HTTP/1.1 413 Payload Too Large";
		body = "413 Payload Too Large: The request is larger than the server is willing or able to process.";
		break;
	case URI_TOO_LONG:
		status_line = "HTTP/1.1 414 URI Too Long";
		body = "414 URI Too Long: The URI provided was too long for the server to process.";
		break;
	case VERSION_NOT_SUPPORTED:
		status_line = "HTTP/1.1 505 HTTP Version Not Supported";
		body = "505 HTTP Version Not Supported: The HTTP version used in the request is not supported by the server.";
		break;
	default:
		status_line = "HTTP/1.1 500 Internal Server Error";
		body = "500 Internal Server Error: The server encountered an unexpected condition that prevented it from fulfilling the request.";
		break;
	}

	std::string response = status_line + "\r\n";
	response += "Content-Type: text/plain\r\n"; // TODO: Create function to create error pages with html and don't return text/plain???

	std::stringstream ss;
	ss << body.size();
	response += "Content-Length: " + ss.str() + "\r\n";

	response += "\r\n";
	response += body;

	return response;
}

std::string Server::_processRequest(int clientfd)
{
	std::string request = _readRequest(clientfd);
	std::cout << request << std::endl;

	std::string method, requestedFile, body;
	std::map<std::string, std::string> headers;
	ParseRequestError error = this->_parseRequest(request, method, requestedFile, headers, body);
	if (error != NO_ERROR)
		return this->_generateErrorResponse(error);

	std::string response = this->_processResponse(method, requestedFile);
	return response;
}

std::string Server::_handleMethods(const std::string &method, const std::string &requestedFile)
{
	std::string response;

	if (method == "GET")
		response = this->_handleGET(requestedFile);
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
	if (filename.find(".html") != std::string::npos || filename.find(".htm") != std::string::npos)
		return "text/html";
	else if (filename.find(".css") != std::string::npos)
		return "text/css";
	else if (filename.find(".js") != std::string::npos)
		return "application/javascript";
	else if (filename.find(".json") != std::string::npos)
		return "application/json";
	else if (filename.find(".xml") != std::string::npos)
		return "application/xml";
	else if (filename.find(".pdf") != std::string::npos)
		return "application/pdf";
	else if (filename.find(".jpg") != std::string::npos || filename.find(".jpeg") != std::string::npos)
		return "image/jpeg";
	else if (filename.find(".png") != std::string::npos)
		return "image/png";
	else if (filename.find(".gif") != std::string::npos)
		return "image/gif";
	else if (filename.find(".svg") != std::string::npos)
		return "image/svg+xml";
	else if (filename.find(".ico") != std::string::npos)
		return "image/x-icon";
	else if (filename.find(".tif") != std::string::npos || filename.find(".tiff") != std::string::npos)
		return "image/tiff";
	else if (filename.find(".webp") != std::string::npos)
		return "image/webp";
	else if (filename.find(".mp3") != std::string::npos)
		return "audio/mpeg";
	else if (filename.find(".wav") != std::string::npos)
		return "audio/wav";
	else if (filename.find(".mp4") != std::string::npos)
		return "video/mp4";
	else if (filename.find(".avi") != std::string::npos)
		return "video/x-msvideo";
	else if (filename.find(".mpeg") != std::string::npos || filename.find(".mpg") != std::string::npos)
		return "video/mpeg";
	else if (filename.find(".webm") != std::string::npos)
		return "video/webm";
	else if (filename.find(".zip") != std::string::npos)
		return "application/zip";
	else if (filename.find(".tar") != std::string::npos)
		return "application/x-tar";
	else if (filename.find(".gz") != std::string::npos || filename.find(".gzip") != std::string::npos)
		return "application/gzip";
	else if (filename.find(".txt") != std::string::npos)
		return "text/plain";
	else if (filename.find(".rtf") != std::string::npos)
		return "application/rtf";
	else if (filename.find(".doc") != std::string::npos || filename.find(".docx") != std::string::npos)
		return "application/msword";
	else if (filename.find(".xls") != std::string::npos || filename.find(".xlsx") != std::string::npos)
		return "application/vnd.ms-excel";
	else if (filename.find(".ppt") != std::string::npos || filename.find(".pptx") != std::string::npos)
		return "application/vnd.ms-powerpoint";
	else
		return "application/octet-stream"; // Default binary file type
}

std::string Server::_handleGET(const std::string &requestedFile)
{
	// TODO: Refactor and check the requested file here?????
	// TODO: Refactor and create enums, generation of error codes like requests differently...???
	std::string response;
	std::string file;
	std::string status;

	std::cout << "Requested file: " << requestedFile << std::endl;
	if (requestedFile[requestedFile.size() - 1] == '/') // If is a directory just serve the index.html or index.htm
	{
		if (access(("pages" + requestedFile + "index.html").c_str(), F_OK) == 0)
			file = "pages" + requestedFile + "index.html";
		else if (access(("pages" + requestedFile + "index.htm").c_str(), F_OK) == 0)
			file = "pages" + requestedFile + "index.htm";
	}
	else
		file = "pages" + requestedFile;

	// Check if the file has an extension, if not add .html
	if (file.find_last_of(".") == std::string::npos)
		file += ".html";

	// Check if the file exists
	if (access(file.c_str(), F_OK) != 0)
	{
		status = "404 Not Found";
		file = "pages/404.html";
	}
	else
	{
		// Check if the file is readable
		if (access(file.c_str(), R_OK) != 0)
		{
			status = "403 Forbidden";
			file = "pages/403.html";
		}
		else
			status = "200 OK";
	}

	std::string fileContent = this->_readFile(file);
	std::ostringstream ss;
	ss << fileContent.size();
	std::string contentType = this->_determineContentType(file);
	if (status.empty())
		status = "200 OK";
	response = "HTTP/1.1 " + status + "\r\n" +
			   "Content-Type: " + contentType + "\r\n" +
			   "Content-Length: " + ss.str() + "\r\n\r\n" + fileContent;

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

// TODO: close all the sockets and the server socket when we finish the server when we CTRL+C???? Check it because port is still in use after CTRL+C during some time
