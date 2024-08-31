/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GlobalConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 19:19:30 by isromero          #+#    #+#             */
/*   Updated: 2024/08/31 12:42:31 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GLOBALCONFIG_HPP
#define GLOBALCONFIG_HPP

#include "includes.hpp"
#include "ServerConfig.hpp"
#include "utils.hpp"

class GlobalConfig
{
private:
	std::vector<ServerConfig> _servers;

	void _parseConfigFile(const std::string &filePath);
	void _parseServerParameters(ServerConfig &currentServer, const std::string &param, std::string &value);
	void _parseLocationBlock(ServerConfig &currentServer, const std::string &locationPath, const std::vector<std::string> &locationBlock);

public:
	GlobalConfig(const std::string &configFilePath);
	~GlobalConfig();

	int getMainPort() const;
	const std::pair<bool, ServerConfig> getServerConfig(const std::pair<std::string, int> &hostInfo, const std::pair<std::string, int> &destInfo) const;
};

#endif
