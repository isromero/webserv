/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 12:21:58 by isromero          #+#    #+#             */
/*   Updated: 2024/08/25 12:07:56 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket(ServerConfig config) : _config(config), _serverfd(-1) {}

Socket::~Socket()
{
	if (this->_serverfd != -1)
		close(this->_serverfd);
}

void Socket::_createSocket()
{
	// AF_INET: ipv4
	// SOCK_STREAM: TCP
	// 0: Default (0 = TCP)
	this->_serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_serverfd < 0)
		throw std::runtime_error("Error: creating the socket: " + std::string(strerror(errno)));
}

void Socket::_configureSocket()
{
	// Set the socket to reuse the address for restart and avoid the "Address already in use" error
	int enable = 1;
	if (setsockopt(this->_serverfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
		throw std::runtime_error("Error: setting the socket options: " + std::string(strerror(errno)));
}

void Socket::_bindSocket()
{
	// Create the server address
	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress)); // Good practice
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(this->_config.getPort()); // htons converts the port number to network byte order

	std::string host = this->_config.getHost();
	if (host == "0.0.0.0")
		serverAddress.sin_addr.s_addr = INADDR_ANY; // Listen in any address
	else if (host == "localhost" || host == "127.0.0.1")
		serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // Listen in localhost
	else
		serverAddress.sin_addr.s_addr = inet_addr(host.c_str());

	// Bind the socket to the address and port
	if (bind(this->_serverfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
		throw std::runtime_error("Error: binding the socket: " + std::string(strerror(errno)));
}

void Socket::_listenSocket()
{
	// Listen for incoming connections
	if (listen(this->_serverfd, MAX_CLIENTS) == -1)
		throw std::runtime_error("Error: listening the socket: " + std::string(strerror(errno)));
}

void Socket::init()
{
	this->_createSocket();
	this->_configureSocket();
	this->_bindSocket();
	this->_listenSocket();
}

int Socket::getServerFd() const
{
	return this->_serverfd;
}
