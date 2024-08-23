/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adgutier <adgutier@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/18 19:01:20 by adgutier          #+#    #+#             */
/*   Updated: 2024/08/18 19:01:20 by adgutier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

ServerConfig::ServerConfig(const std::string &configFilePath) : _port(6969), _host("localhost"), _serverName("localhost"), _root("./"), _indexFile("index.html"), _clientMaxBodySize(1000000), _errorPages(""), _routeRoot(""), _allowedMethods("GET HEAD POST"), _redirect(""), _autoindex(false), _cgiExtension(".php"), _uploadEnable(false), _uploadSavePath("./"), _cgiBinPath("./")
{

	try
	{
		this->_parseConfigFile(configFilePath);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error parsing config file: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

ServerConfig::ServerConfig(const ServerConfig &other) : _port(other._port), _host(other._host), _serverName(other._serverName), _root(other._root), _indexFile(other._indexFile), _clientMaxBodySize(other._clientMaxBodySize), _errorPages(other._errorPages), _routeRoot(other._routeRoot), _allowedMethods(other._allowedMethods), _redirect(other._redirect), _autoindex(other._autoindex), _cgiExtension(other._cgiExtension), _uploadEnable(other._uploadEnable), _uploadSavePath(other._uploadSavePath), _cgiBinPath(other._cgiBinPath) {}

ServerConfig::~ServerConfig() {}

void ServerConfig::_parseConfigFile(const std::string &filePath)
{
	std::ifstream configFile(filePath.c_str());
	if (!configFile.is_open())
		throw std::runtime_error("Error opening the config file");

	std::string line;
	int lineNum = 0;
	bool isServerBlock = false;

	while (std::getline(configFile, line))
	{
		++lineNum;
		trim(line);
		trimTabs(line);

		if (line.empty() || line[0] == '#') // Empty line or comment
			continue;

		if (line == "server {")
		{
			isServerBlock = true;
			continue;
		}

		if (line == "}")
		{
			isServerBlock = false;
			continue;
		}

		if (!isServerBlock)
			throw std::runtime_error("Config outside server block at line " + toString(lineNum));

		if (line.find("location") == 0) // Find returns 0 if the string starts with "location"
		{
			std::string locationPath = line.substr(9); // Skip "location "
			trim(locationPath);
			trimTabs(locationPath);
			locationPath.substr(0, locationPath.size() - 1); // Remove the {

			std::vector<std::string> locationBlock;
			while (std::getline(configFile, line))
			{
				++lineNum;
				trim(line);
				trimTabs(line);

				if (line.empty() || line[0] == '#') // Empty line or comment
					continue;

				if (line == "}") // End of location block
					break;

				locationBlock.push_back(line);
			}
			try
			{
				this->_parseLocationBlock(locationPath, locationBlock);
			}
			catch (const std::exception &e)
			{
				throw std::runtime_error("Error parsing location block at line " + toString(lineNum) + ": " + e.what());
			}
		}
	}

	configFile.close();
}

void ServerConfig::_parseLocationBlock(const std::string &locationPath, const std::vector<std::string> &locationBlock)
{
}

int ServerConfig::getPort() const { return this->_port; }
std::string ServerConfig::getServerName() const { return this->_serverName; }
std::string ServerConfig::getHost() const { return this->_host; }
std::string ServerConfig::getRoot() const { return this->_root; }
std::string ServerConfig::getIndex() const { return this->_index; }
size_t ServerConfig::getClientMaxBodySize() const { return this->_clientMaxBodySize; }
