/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 12:21:58 by isromero          #+#    #+#             */
/*   Updated: 2024/09/01 19:41:01 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket(GlobalConfig config) : _globalConfig(config), _serverfds() {}

Socket::~Socket()
{
	for (std::vector<int>::iterator it = this->_serverfds.begin(); it != this->_serverfds.end(); ++it)
		close(*it);
}

int Socket::_createSocket()
{
	// AF_INET: ipv4
	// SOCK_STREAM: TCP
	// 0: Default (0 = TCP)
	int serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverfd < 0)
		throw std::runtime_error("Error: creating the socket: " + std::string(strerror(errno)));

	return serverfd;
}

void Socket::_configureSocket(int serverfd)
{
	// Set the socket to reuse the address for restart and avoid the "Address already in use" error
	int enable = 1;
	if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
		throw std::runtime_error("Error: setting the socket options: " + std::string(strerror(errno)));
}

void Socket::_bindSocket(int serverfd, std::vector<ServerConfig>::const_iterator it)
{
	// Create the server address
	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress)); // Good practice
	serverAddress.sin_family = AF_INET;				  // IPv4
	serverAddress.sin_port = htons(it->getPort());	  // Convert the port to network byte order

	if (it->getHost() == "0.0.0.0" || it->getHost().empty())
		serverAddress.sin_addr.s_addr = INADDR_ANY;
	else if (it->getHost() == "localhost" || it->getHost() == "127.0.0.1")
		serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	else
	{
		if (inet_pton(AF_INET, it->getHost().c_str(), &serverAddress.sin_addr) <= 0)
			throw std::runtime_error("Error: converting the IP address: " + std::string(strerror(errno)));
	}

	// Bind the socket to the address and port
	if (bind(serverfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
		throw std::runtime_error("Error: binding the socket: " + std::string(strerror(errno)));
}

void Socket::_listenSocket(int serverfd)
{
	// Listen for incoming connections
	if (listen(serverfd, MAX_CLIENTS) == -1)
		throw std::runtime_error("Error: listening the socket: " + std::string(strerror(errno)));
}

void Socket::createSockets()
{
	const std::vector<ServerConfig> &servers = this->_globalConfig.getServers();
	for (std::vector<ServerConfig>::const_iterator it = servers.begin(); it != servers.end(); ++it)
	{
		int serverfd = this->_createSocket();
		this->_configureSocket(serverfd);
		this->_bindSocket(serverfd, it);
		this->_listenSocket(serverfd);

		this->_serverfds.push_back(serverfd);
	}
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

std::vector<int> Socket::getServerFds() const { return this->_serverfds; }
