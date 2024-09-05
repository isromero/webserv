/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 12:20:23 by isromero          #+#    #+#             */
/*   Updated: 2024/09/01 17:25:13 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "includes.hpp"
#include "GlobalConfig.hpp"

#define MAX_CLIENTS 10000

class Socket
{
private:
	GlobalConfig _globalConfig;
	std::vector<int> _serverfds; // ! We use one instance of socket but handle multiple serverfds for each server block

	int _createSocket();
	void _configureSocket(int serverfd);
	void _bindSocket(int serverfd, std::vector<ServerConfig>::const_iterator it);
	void _listenSocket(int serverfd);

public:
	Socket(GlobalConfig config);
	~Socket();

	void createSockets();
	std::pair<std::string, int> getDestinationInfo(int clientfd);

	std::vector<int> getServerFds() const;
};

#endif
