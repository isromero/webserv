/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 20:01:23 by isromero          #+#    #+#             */
/*   Updated: 2024/08/28 20:01:23 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

ServerConfig::ServerConfig() : _port(6969), _serverNames(), _host("0.0.0.0"), _root("./var/www/html"), _indexes(), _clientMaxBodySize(1024 * 1024), _errorPages(), _locations()
{
}

ServerConfig &ServerConfig::operator=(const ServerConfig &other)
{
	if (this != &other)
	{
		this->_port = other._port;
		this->_serverNames = other._serverNames;
		this->_host = other._host;
		this->_root = other._root;
		this->_indexes = other._indexes;
		this->_clientMaxBodySize = other._clientMaxBodySize;
		this->_errorPages = other._errorPages;
		this->_locations = other._locations;
	}
	return *this;
}

ServerConfig::~ServerConfig() {}

bool ServerConfig::isMethodAllowed(const std::vector<LocationConfig> &locations, const std::string &path, const std::string &method) const
{
	const std::string mainPath = extractMainPath(path);

	// Find the best match for the path(best match is the longest path that matches the beginning of the request path)
	// This prevents errors like "/" matching before "/route" when path is "/route"
	for (std::vector<LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it)
	{
		if (mainPath == it->path)
		{
			if (it->allowedMethods.empty()) // If no methods are specified in the block, all methods are allowed
				return true;
			for (std::vector<std::string>::const_iterator it2 = it->allowedMethods.begin(); it2 != it->allowedMethods.end(); ++it2)
			{
				if (*it2 == method)
					return true;
			}
			return false; // If the path matches but the method does not, return false
		}
	}
	return true; // If no location block matches, all methods are allowed by default
}

bool ServerConfig::isAutoindex(const std::vector<LocationConfig> &locations, const std::string &path) const
{
	const std::string mainPath = extractMainPath(path);

	for (std::vector<LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it)
	{
		if (mainPath == it->path)
			return it->autoindex;
	}
	return false; // If no location block matches, autoindex is off by default
}

const std::string ServerConfig::getUploadDir(const std::string &path) const
{
	const std::string mainPath = extractMainPath(path);

	for (std::vector<LocationConfig>::const_iterator it = this->_locations.begin(); it != this->_locations.end(); ++it)
	{
		if (mainPath == it->path)
			return it->uploadDir;
	}
	return "./var/www/uploads"; // If no location block matches, return the default upload directory
}

const std::string ServerConfig::getRedirect(const std::string &path) const
{
	const std::string mainPath = extractMainPath(path);

	for (std::vector<LocationConfig>::const_iterator it = this->_locations.begin(); it != this->_locations.end(); ++it)
	{
		if (mainPath == it->path)
			return it->redirect;
	}
	return ""; // If no location block matches, return an empty string
}

const std::string ServerConfig::getLocationPath(const std::string &path) const
{
	const std::string mainPath = extractMainPath(path);

	for (std::vector<LocationConfig>::const_iterator it = this->_locations.begin(); it != this->_locations.end(); ++it)
	{
		if (mainPath == it->path)
			return it->path;
	}
	return ""; // If no location block matches, return an empty string
}

const std::string ServerConfig::getLocationCGIPath(const std::string &path) const
{
	std::string mainPath = path;
	const std::string pathInfo = extractPathInfo(mainPath);
	const std::string queryString = extractQueryString(mainPath);
	mainPath = extractCGIMainPath(mainPath);

	for (std::vector<LocationConfig>::const_iterator it = this->_locations.begin(); it != this->_locations.end(); ++it)
	{
		if (mainPath == it->path)
			return it->path;
	}
	return ""; // If no location block matches, return an empty string
}

const std::string ServerConfig::getCGIExtension(const std::string &mainPath) const
{
	for (std::vector<LocationConfig>::const_iterator it = this->_locations.begin(); it != this->_locations.end(); ++it)
	{
		if (mainPath == it->path)
			return it->cgiExtension;
	}
	return ".cgi"; // If no location block matches, return the default CGI extension
}

const std::string ServerConfig::getCGIBin(const std::string &path) const
{
	const std::string mainPath = extractMainPath(path);

	for (std::vector<LocationConfig>::const_iterator it = this->_locations.begin(); it != this->_locations.end(); ++it)
	{
		if (mainPath == it->path)
			return it->cgiBin;
	}
	return "./var/www/cgi-bin"; // If no location block matches, return the default CGI bin directory
}

void ServerConfig::setPort(int port) { this->_port = port; }
void ServerConfig::addServerName(const std::string &serverName) { this->_serverNames.push_back(serverName); }
void ServerConfig::setHost(const std::string &host) { this->_host = host; }
void ServerConfig::setRoot(const std::string &root) { this->_root = root; }
void ServerConfig::addIndex(const std::string &index) { this->_indexes.push_back(index); }
void ServerConfig::setClientMaxBodySize(size_t clientMaxBodySize) { this->_clientMaxBodySize = clientMaxBodySize; }
void ServerConfig::addErrorPage(int errorCode, const std::string &errorPage) { this->_errorPages[errorCode] = errorPage; }
void ServerConfig::addLocation(const LocationConfig &location) { this->_locations.push_back(location); }

int ServerConfig::getPort() const { return this->_port; }
std::vector<std::string> ServerConfig::getServerNames() const { return this->_serverNames; }
std::string ServerConfig::getHost() const { return this->_host; }
std::string ServerConfig::getRoot() const { return this->_root; }
std::vector<std::string> ServerConfig::getIndexes() const { return this->_indexes; }
size_t ServerConfig::getClientMaxBodySize() const { return this->_clientMaxBodySize; }
std::map<int, std::string> ServerConfig::getErrorPages() const { return this->_errorPages; }
std::vector<LocationConfig> ServerConfig::getLocations() const { return this->_locations; }
