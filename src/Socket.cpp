/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 12:21:58 by isromero          #+#    #+#             */
/*   Updated: 2024/08/31 12:16:30 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket(GlobalConfig config) : _globalConfig(config), _serverfd(-1) {}

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
	serverAddress.sin_port = htons(this->_globalConfig.getMainPort()); // htons converts the port number to network byte order
	serverAddress.sin_addr.s_addr = INADDR_ANY;						   // ! We accept connections from any IP BUT then the server choose the configs of the server blocks that match the host

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

// Get the IP and port of the destination for choose the server block because we create only one Socket for all the server blocks
std::pair<std::string, int> Socket::getDestinationInfo(int clientfd)
{
	struct sockaddr_in clientAddress;
	socklen_t clientAddressSize = sizeof(clientAddress);

	if (getsockname(clientfd, (struct sockaddr *)&clientAddress, &clientAddressSize) == -1)
		throw std::runtime_error("Error: getting the destination info: " + std::string(strerror(errno)));

	char destIP[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &(clientAddress.sin_addr), destIP, INET_ADDRSTRLEN) == NULL) // Convert the IP address to a string and store it in destIP (INET_ADDRSTRLEN is the max length of the string)
		throw std::runtime_error("Error: converting the IP address to a string: " + std::string(strerror(errno)));

	return std::make_pair(std::string(destIP), ntohs(clientAddress.sin_port));
}

int Socket::getServerFd() const
{
	return this->_serverfd;
}
