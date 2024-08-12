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

#if defined(__linux__)
#include <sys/epoll.h>
#elif defined(__APPLE__)
#include <sys/event.h>
#endif

#define MAX_CLIENTS 10000
#define MAX_EVENTS 10000

class Server
{
private:
	int _port;
	int _serverfd;

	void _createSocket();
	void _bindSocket();
	void _listenSocket();
	int _acceptClient();
	std::string _processRequest(int clientfd);
	std::string _processResponse(const std::string &request);
	std::string _handleMethods(const std::string &method, const std::string &requestedFile);
	std::string _readFile(const std::string &filename);
	std::string _determineContentType(const std::string &filename);
	std::string _handleGET(const std::string &requestedFile);
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
