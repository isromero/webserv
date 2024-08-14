/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 12:20:23 by isromero          #+#    #+#             */
/*   Updated: 2024/08/14 12:47:45 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

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
