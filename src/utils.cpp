/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:29:36 by isromero          #+#    #+#             */
/*   Updated: 2024/08/18 14:22:31 by isromero         ###   ########.fr       */
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

bool saveFile(const std::string &content, const std::string &filename, std::string &savedPath)
{
	std::string filepath = "pages/uploads/" + getFilenameAndDate(filename);
	std::ofstream file(filepath.c_str(), std::ios::binary);
	if (file.is_open())
	{
		file.write(content.c_str(), content.size());
		file.close();
		savedPath = filepath;
		return true;
	}
	return false;
}
