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
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <cstdlib>
#include <map>

#if defined(__linux__)
#include <sys/epoll.h>
#elif defined(__APPLE__)
#include <sys/event.h>
#endif

#define MAX_EVENTS 10000

#include "status.hpp"
#include "utils.hpp"
#include "Socket.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "ServerConfig.hpp"

class Server
{
private:
	Socket _socket;
	ServerConfig _config;

	int _acceptClient();
	std::string _processRequestResponse(int clientfd);
	void _sendResponse(int clientfd, const std::string &response);

public:
	Server(const std::string &configFilePath);
	Server(int port);
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
