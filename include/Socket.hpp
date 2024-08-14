/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adgutier <adgutier@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 12:20:23 by isromero          #+#    #+#             */
/*   Updated: 2024/08/14 18:06:14 by adgutier         ###   ########.fr       */
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


#define MAX_CLIENTS 10000

class Socket
{
private:
	int _port;
	int _serverfd;

	void _createSocket();
	void _configureSocket();
	void _bindSocket();
	void _listenSocket();

public:
	Socket(int port);
	~Socket();

	void init();
	int getPort() const;
	int getServerFd() const;
};

#endif
