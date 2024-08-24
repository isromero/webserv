/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 12:20:23 by isromero          #+#    #+#             */
/*   Updated: 2024/08/24 11:01:18 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

#include "ServerConfig.hpp"

#define MAX_CLIENTS 10000

class Socket
{
private:
	ServerConfig _config;
	int _serverfd;

	void _createSocket();
	void _configureSocket();
	void _bindSocket();
	void _listenSocket();

public:
	Socket(ServerConfig config);
	~Socket();

	void init();
	int getServerFd() const;
};

#endif
