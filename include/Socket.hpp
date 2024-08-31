/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 12:20:23 by isromero          #+#    #+#             */
/*   Updated: 2024/08/31 12:16:10 by isromero         ###   ########.fr       */
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
	int _serverfd;

	void _createSocket();
	void _configureSocket();
	void _bindSocket();
	void _listenSocket();

public:
	Socket(GlobalConfig config);
	~Socket();

	void init();
	std::pair<std::string, int> getDestinationInfo(int clientfd);
	int getServerFd() const;
};

#endif
