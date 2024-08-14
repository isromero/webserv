/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 13:16:33 by isromero          #+#    #+#             */
/*   Updated: 2024/08/12 14:37:40 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <cstdlib>
#include <map>

#if defined(__linux__)
#include <sys/epoll.h>
#elif defined(__APPLE__)
#include <sys/event.h>
#endif

#define MAX_CLIENTS 10000
#define MAX_EVENTS 10000

enum ParseRequestError
{
	NO_ERROR,
	INVALID_REQUEST,
	INVALID_REQUEST_LINE,
	INVALID_HEADER_FORMAT,
	INCOMPLETE_BODY,
	INVALID_REQUEST_TARGET,
	INVALID_METHOD,
	INVALID_CONTENT_LENGTH,
	PAYLOAD_TOO_LARGE,
	URI_TOO_LONG,
	VERSION_NOT_SUPPORTED,
};

class Server
{
private:
	int _port;
	int _serverfd;

	void _createSocket();
	void _bindSocket();
	void _listenSocket();
	int _acceptClient();
	std::string _readRequest(int clientfd);
	std::string _secureFilePath(const std::string &path);
	ParseRequestError _parseRequest(const std::string &request, std::string &method, std::string &requestedFile, std::map<std::string, std::string> &headers, std::string &body);
	std::string _processRequest(int clientfd);
	std::string _processResponse(const std::string &method, const std::string &requestedFile, const std::string &request);
	std::string _handleMethods(const std::string &method, const std::string &requestedFile, const std::string &request);
	std::string _generateErrorResponse(ParseRequestError error);
	std::string _readFile(const std::string &filename);
	std::string _determineContentType(const std::string &filename);
	std::string _handleGET(const std::string &requestedFile);
	std::string _handlePOST(const std::string &request);
	void _sendResponse(int clientfd, const std::string &response);

public:
	Server();
	Server(int port);
	Server(const std::string &configFile);
	Server(const Server &other);
	Server &operator=(const Server &other);
	~Server();

#if defined(__linux__)
	void runLinux();
#elif defined(__APPLE__)
	void runMac();
#endif
};

#endif
