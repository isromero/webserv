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

ServerConfig::ServerConfig(const std::string &configFilePath) : _port(6969), _serverNames(), _host("0.0.0.0"), _root("/var/www/html"), _indexes(), _clientMaxBodySize(1024 * 1024), _errorPages(), _locations()
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

ServerConfig::ServerConfig(const ServerConfig &other) : _port(other._port), _serverNames(other._serverNames), _host(other._host), _root(other._root), _indexes(other._indexes), _clientMaxBodySize(other._clientMaxBodySize), _errorPages(other._errorPages), _locations(other._locations) {}

ServerConfig::~ServerConfig() {}

static const std::string extractMainPath(const std::string &fullPath)
{
	size_t lastSlash = fullPath.find_last_of('/');
	if (lastSlash == 0)
		return "/";
	return fullPath.substr(0, lastSlash + 1); // Include the last slash
}

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
	if (param == "listen")
	{
		this->_port = std::atoi(value.c_str());
		if (this->_port < 0 || this->_port > 65535)
			throw std::runtime_error("Invalid port: " + value);
	}
	else if (param == "server_name")
	{
		std::istringstream iss(value);
		while (iss >> value)
			this->_serverNames.push_back(value);
	}
	else if (param == "host")
		this->_host = value;
	else if (param == "root")
	{
		this->_root = "." + value;
		if (access(this->_root.c_str(), F_OK) == -1)
			throw std::runtime_error("Root directory does not exist: " + this->_root);
	}
	else if (param == "index")
	{
		std::istringstream iss(value);
		while (iss >> value)
			this->_indexes.push_back(value);
	}
	else if (param == "client_max_body_size")
	{
		std::string sizeUnit = value.substr(value.size() - 1);

		size_t size = std::atoi(value.substr(0, value.size() - 1).c_str());
		if (sizeUnit == "k" || sizeUnit == "K")
			this->_clientMaxBodySize = size * 1024;
		else if (sizeUnit == "m" || sizeUnit == "M")
			this->_clientMaxBodySize = size * 1024 * 1024;
		else if (sizeUnit == "g" || sizeUnit == "G")
			this->_clientMaxBodySize = size * 1024 * 1024 * 1024;
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
		this->_errorPages[errorCode] = this->_root + errorPage;
		if (access(this->_errorPages[errorCode].c_str(), F_OK) == -1)
			throw std::runtime_error("Error page does not exist: " + this->_errorPages[errorCode]);
	}
	else
		throw std::runtime_error("Unknown server parameter: " + param);
}

static void removeSemicolon(std::string &str)
{
	if (str[str.size() - 1] == ';')
		str = str.substr(0, str.size() - 1);
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

	this->_locations.push_back(location);
}

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
	return "/var/www/uploads"; // If no location block matches, return the default upload directory
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

int ServerConfig::getPort() const { return this->_port; }
std::vector<std::string> ServerConfig::getServerNames() const { return this->_serverNames; }
std::string ServerConfig::getHost() const { return this->_host; }
std::string ServerConfig::getRoot() const { return this->_root; }
std::vector<std::string> ServerConfig::getIndexes() const { return this->_indexes; }
size_t ServerConfig::getClientMaxBodySize() const { return this->_clientMaxBodySize; }
std::map<int, std::string> ServerConfig::getErrorPages() const { return this->_errorPages; }
std::vector<LocationConfig> ServerConfig::getLocations() const { return this->_locations; }
