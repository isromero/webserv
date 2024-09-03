/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 20:01:32 by isromero          #+#    #+#             */
/*   Updated: 2024/08/28 20:01:32 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "includes.hpp"
#include "utils.hpp"

struct LocationConfig
{
	std::string path;
	std::vector<std::string> allowedMethods;
	bool autoindex;
	std::string uploadDir;
	std::string cgiExtension;
	std::string cgiBin;
	std::string redirect;
};

class ServerConfig
{
private:
	int _port;
	std::vector<std::string> _serverNames;
	std::string _host;
	std::string _root;
	std::vector<std::string> _indexes;
	size_t _clientMaxBodySize;
	std::map<int, std::string> _errorPages;
	std::vector<LocationConfig> _locations;

public:
	ServerConfig();
	ServerConfig &operator=(const ServerConfig &other);
	~ServerConfig();

	bool isMethodAllowed(const std::vector<LocationConfig> &location, const std::string &path, const std::string &method) const;
	bool isAutoindex(const std::vector<LocationConfig> &location, const std::string &path) const;
	const std::string getUploadDir(const std::string &path) const;
	const std::string getRedirect(const std::string &path) const;
	const std::string getLocationPath(const std::string &path) const;
	const std::string getLocationCGIPath(const std::string path) const;
	const std::string getCGIExtension(const std::string &mainPath) const;
	const std::string getCGIBin(const std::string &path) const;

	void setPort(int port);
	void addServerName(const std::string &serverName);
	void setHost(const std::string &host);
	void setRoot(const std::string &root);
	void addIndex(const std::string &index);
	void setClientMaxBodySize(size_t clientMaxBodySize);
	void addErrorPage(int errorCode, const std::string &errorPage);
	void addLocation(const LocationConfig &location);

	int getPort() const;
	std::vector<std::string> getServerNames() const;
	std::string getHost() const;
	std::string getRoot() const;
	std::vector<std::string> getIndexes() const;
	size_t getClientMaxBodySize() const;
	std::map<int, std::string> getErrorPages() const;
	std::vector<LocationConfig> getLocations() const;
};

#endif
