/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adgutier <adgutier@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/18 18:19:00 by adgutier          #+#    #+#             */
/*   Updated: 2024/08/18 18:19:00 by adgutier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include "utils.hpp"

struct LocationConfig
{
	std::string path;
	std::vector<std::string> allowedMethods;
	bool autoindex;
	std::string uploadDir;
	std::string cgiExtension;
	std::string cgiBin;
};

class ServerConfig
{
private:
	int _port;
	std::vector<std::string> _serverNames;
	std::string _host; // TODO: listen in X host in Socket??? Same like host header os is the serverName???
	std::string _root;
	std::string _index;
	size_t _clientMaxBodySize;
	std::map<int, std::string> _errorPages;
	std::vector<LocationConfig> _locations;

	void _parseConfigFile(const std::string &filePath);
	void _parseServerParameters(const std::string &param, std::string &value);
	void _parseLocationBlock(const std::string &locationPath, const std::vector<std::string> &locationBlock);

public:
	ServerConfig(const std::string &configFilePath);
	ServerConfig(const ServerConfig &other);
	~ServerConfig();

	int getPort() const;
	std::vector<std::string> getServerNames() const;
	std::string getHost() const;
	std::string getRoot() const;
	std::string getIndex() const;
	size_t getClientMaxBodySize() const;
	std::map<int, std::string> getErrorPages() const;
	std::vector<LocationConfig> getLocations() const;
};

#endif
