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
#include "utils.hpp"

class ServerConfig
{
private:
    int _port;
    std::string _host;
    std::string _serverName;
    std::string _root;
    std::string _indexFile;
    size_t _clientMaxBodySize;
    std::string _errorPages; // Cambia esto según el formato que estés utilizando
    std::string _routeRoot; // Cambia esto según el formato que estés utilizando
    std::string _allowedMethods; // Cambia esto según el formato que estés utilizando
    std::string _redirect; // Cambia esto según el formato que estés utilizando
    bool _autoindex;
    std::string _cgiExtension; // Cambia esto según el formato que estés utilizando
    bool _uploadEnable;
    std::string _uploadSavePath;
    std::string _cgiBinPath;

public:
ServerConfig(){};
// Getters
    int getPort() const;
    std::string getHost() const;
    std::string getServerName() const;
    std::string getRoot() const;
    std::string getIndexFile() const;
    size_t getClientMaxBodySize() const;
    std::string getErrorPages() const;
    std::string getRouteRoot() const;
    std::string getAllowedMethods() const;
    std::string getRedirect() const;
    bool getAutoindex() const;
    std::string getCgiExtension() const;
    bool getUploadEnable() const;
    std::string getUploadSavePath() const;
    std::string getCgiBinPath() const;

    // Setters
    void setPort(int port);
    void setHost(const std::string &host);
    void setServerName(const std::string &serverName);
    void setRoot(const std::string &root);
    void setIndexFile(const std::string &indexFile);
    void setClientMaxBodySize(size_t size);
    void setErrorPages(const std::string &errorPages);
    void setRouteRoot(const std::string &routeRoot);
    void setAllowedMethods(const std::string &allowedMethods);
    void setRedirect(const std::string &redirect);
    void setAutoindex(bool autoindex);
    void setCgiExtension(const std::string &cgiExtension);
    void setUploadEnable(bool uploadEnable);
    void setUploadSavePath(const std::string &uploadSavePath);
    void setCgiBinPath(const std::string &cgiBinPath);
};

void parseConfigFile(const std::string &filePath, ServerConfig &config);

#endif
