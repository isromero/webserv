/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GlobalConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 19:20:21 by isromero          #+#    #+#             */
/*   Updated: 2024/09/04 21:36:56 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GlobalConfig.hpp"

GlobalConfig::GlobalConfig(const std::string &configFilePath) : _servers()
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

GlobalConfig::~GlobalConfig() {}

static void removeSemicolon(std::string &str)
{
	if (str[str.size() - 1] == ';')
		str = str.substr(0, str.size() - 1);
}

void GlobalConfig::_parseConfigFile(const std::string &filePath)
{
	std::ifstream configFile(filePath.c_str());
	if (!configFile.is_open())
		throw std::runtime_error("Error opening the config file");

	std::string line;
	int lineNum = 0;
	bool isServerBlock = false;
	ServerConfig currentServer;

	while (std::getline(configFile, line))
	{
		++lineNum;
		trim(line);
		trimTabs(line);

		if (line.empty() || line[0] == '#') // Empty line or comment
			continue;

		if (line == "server {")
		{
			if (isServerBlock)
				throw std::runtime_error("Nested server blocks are not allowed at line " + toString(lineNum));
			isServerBlock = true;
			currentServer = ServerConfig(); // Create a new server (virtual host)
			continue;
		}

		if (line == "}")
		{
			if (!isServerBlock)
				throw std::runtime_error("Unexpected } at line " + toString(lineNum));
			isServerBlock = false;
			this->_servers.push_back(currentServer); // Add the current server to the list of servers
			continue;
		}

		if (!isServerBlock)
			throw std::runtime_error("Config outside server block at line " + toString(lineNum));

		if (line.find("location") == 0) // Find returns 0 if the string starts with "location"
		{
			std::string locationPath = line.substr(9); // Skip "location "
			trim(locationPath);
			trimTabs(locationPath);
			locationPath = locationPath.substr(0, locationPath.size() - 1); // Remove the {
			trim(locationPath);												// Trim the spaces between the path and the {

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
				this->_parseLocationBlock(currentServer, locationPath, locationBlock);
			}
			catch (const std::exception &e)
			{
				throw std::runtime_error("Error parsing location block at line " + toString(lineNum) + ": " + e.what());
			}
		}
		else
		{
			std::istringstream iss(line);
			std::string param;
			iss >> param;

			std::string value;
			std::getline(iss, value);
			trim(value);
			trimTabs(value);

			if (!value.empty() && value[value.size() - 1] == ';')
				value = value.substr(0, value.size() - 1);

			try
			{
				this->_parseServerParameters(currentServer, param, value);
			}
			catch (const std::exception &e)
			{
				throw std::runtime_error("Error parsing server parameter at line " + toString(lineNum) + ": " + e.what());
			}
		}
	}

	configFile.close();
}

void GlobalConfig::_parseServerParameters(ServerConfig &currentServer, const std::string &param, std::string &value)
{
	if (param == "listen")
	{
		currentServer.setPort(std::atoi(value.c_str()));
		if (currentServer.getPort() < 0 || currentServer.getPort() > 65535)
			throw std::runtime_error("Invalid port: " + value);
	}
	else if (param == "server_name")
	{
		std::istringstream iss(value);
		while (iss >> value)
			currentServer.addServerName(value);
	}
	else if (param == "host")
	{
		if (value == "localhost")
			currentServer.setHost("127.0.0.1");
		else
			currentServer.setHost(value);
	}
	else if (param == "root")
	{
		const std::string root = "./" + value;
		currentServer.setRoot(root);
		if (access(currentServer.getRoot().c_str(), F_OK) == -1)
			throw std::runtime_error("Root directory does not exist: " + currentServer.getRoot());
	}
	else if (param == "index")
	{
		std::istringstream iss(value);
		while (iss >> value)
			currentServer.addIndex(value);
	}
	else if (param == "client_max_body_size")
	{
		std::string sizeUnit = value.substr(value.size() - 1);

		size_t size = std::atoi(value.substr(0, value.size() - 1).c_str());
		if (sizeUnit == "k" || sizeUnit == "K")
			currentServer.setClientMaxBodySize(size * 1024);
		else if (sizeUnit == "m" || sizeUnit == "M")
			currentServer.setClientMaxBodySize(size * 1024 * 1024);
		else if (sizeUnit == "g" || sizeUnit == "G")
			currentServer.setClientMaxBodySize(size * 1024 * 1024 * 1024);
		else
			throw std::runtime_error("Invalid size unit: " + sizeUnit);
	}
	else if (param == "error_page")
	{
		std::istringstream iss(value);
		int errorCode;
		std::string errorPage;
		iss >> errorCode >> errorPage;
		if (errorCode < 400 || errorCode > 599)
			throw std::runtime_error("Invalid error code: " + toString(errorCode));
		const std::string errorPagePath = currentServer.getRoot() + errorPage;
		if (access(errorPagePath.c_str(), F_OK) == -1)
			throw std::runtime_error("Error page does not exist: " + errorPagePath);
		currentServer.addErrorPage(errorCode, errorPagePath);
	}
	else
		throw std::runtime_error("Unknown server parameter: " + param);
}

void GlobalConfig::_parseLocationBlock(ServerConfig &currentServer, const std::string &locationPath, const std::vector<std::string> &locationBlock)
{
	LocationConfig location;
	location.path = locationPath;

	// locationBlock is a vector of lines
	for (std::vector<std::string>::const_iterator it = locationBlock.begin(); it != locationBlock.end(); ++it)
	{
		std::istringstream iss(*it); // (*it) is a line
		std::string param;
		iss >> param;

		if (param == "allowed_methods")
		{
			std::string method;
			while (iss >> method)
			{
				if (method[method.size() - 1] == ';') // If is the last method
				{
					method = method.substr(0, method.size() - 1);
					location.allowedMethods.push_back(method);
					break;
				}
				location.allowedMethods.push_back(method);
			}
		}
		else if (param == "autoindex")
		{
			std::string value;
			iss >> value;
			removeSemicolon(value);
			location.autoindex = (value == "on");
		}
		else if (param == "upload_dir")
		{
			iss >> location.uploadDir;
			removeSemicolon(location.uploadDir);
			location.uploadDir = "." + location.uploadDir;
			if (access(location.uploadDir.c_str(), F_OK) == -1)
				throw std::runtime_error("Upload directory does not exist: " + location.uploadDir);
		}
		else if (param == "cgi_extension")
		{
			iss >> location.cgiExtension;
			removeSemicolon(location.cgiExtension);
		}
		else if (param == "cgi_bin")
		{
			iss >> location.cgiBin;
			removeSemicolon(location.cgiBin);
			location.cgiBin = "." + location.cgiBin;
			if (access(location.cgiBin.c_str(), F_OK) == -1)
				throw std::runtime_error("CGI bin directory does not exist: " + location.cgiBin);
		}
		else if (param == "redirect")
		{
			iss >> location.redirect;
			removeSemicolon(location.redirect);
		}
		else
			throw std::runtime_error("Unknown location parameter: " + param);
	}

	currentServer.addLocation(location);
}

const std::pair<bool, ServerConfig> GlobalConfig::getServerConfig(const std::pair<std::string, int> &hostInfo, const std::pair<std::string, int> &destInfo) const
{
	ServerConfig defaultServer;
	bool found = false;

	for (std::vector<ServerConfig>::const_iterator it = this->_servers.begin(); it != this->_servers.end(); ++it)
	{
		if (it->getPort() != destInfo.second)
			continue;

		if (it->getHost() != "0.0.0.0" && it->getHost() != destInfo.first)
			continue;

		if (it->getServerNames().empty())
		{
			defaultServer = *it; // Save the default server for this port
			found = true;
			continue;
		}

		const std::vector<std::string> &serverNames = it->getServerNames();
		for (std::vector<std::string>::const_iterator it2 = serverNames.begin(); it2 != serverNames.end(); ++it2)
		{
			if (*it2 == hostInfo.first)
				return std::make_pair(true, *it);
		}
	}

	if (found)
		return std::make_pair(true, defaultServer);

	return std::make_pair(false, defaultServer);
}

const std::vector<ServerConfig> &GlobalConfig::getServers() const { return this->_servers; }
