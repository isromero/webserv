/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:29:36 by isromero          #+#    #+#             */
/*   Updated: 2024/09/02 21:21:47 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

std::string secureFilePath(const std::string &path)
{
	std::string securePath = path;
	size_t pos = 0;
	while ((pos = securePath.find("..")) != std::string::npos)
		securePath.erase(pos, 2);
	while ((pos = securePath.find("//")) != std::string::npos)
		securePath.erase(pos, 1);
	while ((pos = securePath.find("~")) != std::string::npos)
		securePath.erase(pos, 1);
	while ((pos = securePath.find("/./")) != std::string::npos)
		securePath.erase(pos, 2);
	while ((pos = securePath.find("/../")) != std::string::npos)
		securePath.erase(pos, 3);
	return securePath;
}

std::string readFile(const std::string &filename)
{
	std::ifstream file(filename.c_str(), std::ios::binary); // Open the file in binary mode because there's no conversion like this, so is safer
	if (!file.is_open())
	{
		std::cerr << "Error: opening the file: " << strerror(errno) << std::endl;
		return "";
	}
	std::ostringstream ss;
	ss << file.rdbuf();
	file.close();
	return ss.str();
}

std::string getFilenameAndDate(const std::string &originalName)
{
	time_t now = time(0);
	struct tm *ltm = localtime(&now);

	std::ostringstream ss;
	ss << std::setfill('0')
	   << std::setw(4) << (1900 + ltm->tm_year)
	   << std::setw(2) << (1 + ltm->tm_mon)
	   << std::setw(2) << ltm->tm_mday
	   << "_"
	   << std::setw(2) << ltm->tm_hour
	   << std::setw(2) << ltm->tm_min
	   << std::setw(2) << ltm->tm_sec
	   << "_";

	// Get the base name and the extension
	size_t dotPos = originalName.find_last_of(".");
	std::string baseName = (dotPos == std::string::npos) ? originalName : originalName.substr(0, dotPos);
	std::string extension = (dotPos == std::string::npos) ? "" : originalName.substr(dotPos);

	ss << baseName << extension;
	return ss.str();
}

static void ltrim(std::string &s) // Remove leading spaces
{
	std::string::iterator it = s.begin();
	while (it != s.end() && std::isspace(*it))
		++it;
	s.erase(s.begin(), it);
}

static void rtrim(std::string &s) // Remove trailing spaces
{
	std::string::reverse_iterator it = s.rbegin();
	while (it != s.rend() && std::isspace(*it))
		++it;
	s.erase(it.base(), s.end());
}

void trim(std::string &s) // Remove leading and trailing spaces
{
	ltrim(s);
	rtrim(s);
}

void trimTabs(std::string &s) // Remove leading and trailing tabs
{
	std::string::iterator it = s.begin();
	while (it != s.end() && *it == '\t')
		++it;
	s.erase(s.begin(), it);

	std::string::reverse_iterator rit = s.rbegin();
	while (rit != s.rend() && *rit == '\t')
		++rit;
	s.erase(rit.base(), s.end());
}

const std::string extractMainPath(const std::string &fullPath)
{
	size_t lastSlash = fullPath.find_last_of('/');
	if (lastSlash == 0)
		return "/";
	return fullPath.substr(0, lastSlash + 1); // Include the last slash
}

const std::string extractCGIMainPath(const std::string &path)
{
	size_t extensionPos = path.find_last_of('.');
	if (extensionPos == std::string::npos)
		return "";

	// Find the last occurrence of '/'
	size_t lastSlashPos = path.find_last_of('/');
	if (lastSlashPos == std::string::npos || lastSlashPos > extensionPos)
		return "";

	// Extract the main path, excluding the script name
	return path.substr(0, lastSlashPos + 1);
}

const std::string extractQueryString(std::string &scriptName)
{
	std::string queryString = "";

	// Check if there is a query string (?key=value&key2=value2)
	size_t queryPos = scriptName.find('?');
	if (queryPos != std::string::npos) // There is a query string
	{
		size_t queryEnd = scriptName.find_first_of('/', queryPos);
		queryString = scriptName.substr(queryPos + 1, queryEnd - queryPos - 1);
		scriptName = scriptName.substr(0, queryPos);
	}
	return queryString;
}

const std::string extractPathInfo(std::string &scriptName)
{
	std::string pathInfo = "";

	// Get the path info (the part of the URL after the script name and before the query string: /cgi-bin/script.cgi/path/info)
	std::string scriptUntilDot = scriptName.substr(scriptName.find_last_of('.'));
	size_t pathInfoPos = scriptUntilDot.find_first_of('/');
	if (pathInfoPos != std::string::npos)
	{
		pathInfo = scriptUntilDot.substr(pathInfoPos);
		scriptName = scriptName.substr(0, scriptName.find_last_of('/') + 1);
	}
	return pathInfo;
}
