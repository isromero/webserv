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

ServerConfig::ServerConfig(const std::string &configFilePath) : _port(6969), _serverNames(), _host("0.0.0.0"), _root("/var/www/html"), _index("index.html"), _clientMaxBodySize(1024 * 1024), _errorPages(), _locations()
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

ServerConfig::ServerConfig(const ServerConfig &other) : _port(other._port), _serverNames(other._serverNames), _host(other._host), _root(other._root), _index(other._index), _clientMaxBodySize(other._clientMaxBodySize), _errorPages(other._errorPages), _locations(other._locations) {}

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
				this->_parseServerParameters(param, value);
			}
			catch (const std::exception &e)
			{
				throw std::runtime_error("Error parsing server parameter at line " + toString(lineNum) + ": " + e.what());
			}
		}
	}

	configFile.close();
}

void ServerConfig::_parseServerParameters(const std::string &param, std::string &value)
{
	if (param == "port")
	{
		this->_port = std::atoi(value.c_str());
		if (this->_port < 1 || this->_port > 65535)
			throw std::runtime_error("Invalid port number: " + value);
	}
	else if (param == "server_name")
	{
		std::istringstream iss;
		while (iss >> value)
		{
			if (value[value.size() - 1] == ';') // Last server_name
			{
				value = value.substr(0, value.size() - 1);
				this->_serverNames.push_back(value);
				break;
			}
			this->_serverNames.push_back(value);
		}
	}
	else if (param == "host")
		this->_host = value;
	else if (param == "root")
	{
		this->_root = value;
		if (access(this->_root.c_str(), F_OK) == -1)
			throw std::runtime_error("Root directory does not exist: " + this->_root);
	}
	else if (param == "index")
		this->_index = value;
	else if (param == "client_max_body_size")
	{
		this->_clientMaxBodySize = std::atoi(value.c_str());
		if (this->_clientMaxBodySize < 0)
			throw std::runtime_error("Invalid client_max_body_size: " + value);
	}
	else if (param == "error_page")
	{
		std::istringstream iss(value);
		int errorCode;
		std::string errorPage;
		iss >> errorCode >> errorPage;
		if (errorCode < 400 || errorCode > 599)
			throw std::runtime_error("Invalid error code: " + toString(errorCode));
		this->_errorPages[errorCode] = errorPage;
	}
	else
		throw std::runtime_error("Unknown server parameter: " + param);
}

void ServerConfig::_parseLocationBlock(const std::string &locationPath, const std::vector<std::string> &locationBlock)
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
			if (value[value.size() - 1] == ';')
				value = value.substr(0, value.size() - 1);
			location.autoindex = (value == "on");
		}
		else if (param == "upload_dir")
		{
			iss >> location.uploadDir;
			if (location.uploadDir[location.uploadDir.size() - 1] == ';')
				location.uploadDir = location.uploadDir.substr(0, location.uploadDir.size() - 1);
			if (access(location.uploadDir.c_str(), F_OK) == -1)
				throw std::runtime_error("Upload directory does not exist: " + location.uploadDir);
		}
		else if (param == "cgi_extension")
		{
			iss >> location.cgiExtension;
			if (location.cgiExtension[location.cgiExtension.size() - 1] == ';')
				location.cgiExtension = location.cgiExtension.substr(0, location.cgiExtension.size() - 1);
		}
		else if (param == "cgi_bin")
		{
			iss >> location.cgiBin;
			if (location.cgiBin[location.cgiBin.size() - 1] == ';')
				location.cgiBin = location.cgiBin.substr(0, location.cgiBin.size() - 1);
			if (access(location.cgiBin.c_str(), F_OK) == -1)
				throw std::runtime_error("CGI bin directory does not exist: " + location.cgiBin);
		}
		else
			throw std::runtime_error("Unknown location parameter: " + param);
	}

	this->_locations.push_back(location);
}

int ServerConfig::getPort() const { return this->_port; }
std::vector<std::string> ServerConfig::getServerNames() const { return this->_serverNames; }
std::string ServerConfig::getHost() const { return this->_host; }
std::string ServerConfig::getRoot() const { return this->_root; }
std::string ServerConfig::getIndex() const { return this->_index; }
size_t ServerConfig::getClientMaxBodySize() const { return this->_clientMaxBodySize; }
std::map<int, std::string> ServerConfig::getErrorPages() const { return this->_errorPages; }
std::vector<LocationConfig> ServerConfig::getLocations() const { return this->_locations; }
