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

// ServerConfig.cpp

#include "ServerConfig.hpp"

int ServerConfig::getPort() const { return _port; }
std::string ServerConfig::getHost() const { return _host; }
std::string ServerConfig::getServerName() const { return _serverName; }
std::string ServerConfig::getRoot() const { return _root; }
std::string ServerConfig::getIndexFile() const { return _indexFile; }
size_t ServerConfig::getClientMaxBodySize() const { return _clientMaxBodySize; }
std::string ServerConfig::getErrorPages() const { return _errorPages; }
std::string ServerConfig::getRouteRoot() const { return _routeRoot; }
std::string ServerConfig::getAllowedMethods() const { return _allowedMethods; }
std::string ServerConfig::getRedirect() const { return _redirect; }
bool ServerConfig::getAutoindex() const { return _autoindex; }
std::string ServerConfig::getCgiExtension() const { return _cgiExtension; }
bool ServerConfig::getUploadEnable() const { return _uploadEnable; }
std::string ServerConfig::getUploadSavePath() const { return _uploadSavePath; }
std::string ServerConfig::getCgiBinPath() const { return _cgiBinPath; }


void ServerConfig::setPort(int port) { this->_port = port; }
void ServerConfig::setHost(const std::string &host) { _host = host; }
void ServerConfig::setServerName(const std::string &serverName) { _serverName = serverName; }
void ServerConfig::setRoot(const std::string &root) { _root = root; }
void ServerConfig::setIndexFile(const std::string &indexFile) { _indexFile = indexFile; }
void ServerConfig::setClientMaxBodySize(size_t size) { _clientMaxBodySize = size; }
void ServerConfig::setErrorPages(const std::string &errorPages) { _errorPages = errorPages; }
void ServerConfig::setRouteRoot(const std::string &routeRoot) { _routeRoot = routeRoot; }
void ServerConfig::setAllowedMethods(const std::string &allowedMethods) { _allowedMethods = allowedMethods; }
void ServerConfig::setRedirect(const std::string &redirect) { _redirect = redirect; }
void ServerConfig::setAutoindex(bool autoindex) { _autoindex = autoindex; }
void ServerConfig::setCgiExtension(const std::string &cgiExtension) { _cgiExtension = cgiExtension; }
void ServerConfig::setUploadEnable(bool uploadEnable) { _uploadEnable = uploadEnable; }
void ServerConfig::setUploadSavePath(const std::string &uploadSavePath) { _uploadSavePath = uploadSavePath; }
void ServerConfig::setCgiBinPath(const std::string &cgiBinPath) { _cgiBinPath = cgiBinPath;}


int stringToInt(const std::string& str) {
    std::stringstream ss(str);
    int result;
    ss >> result;
    return result;
}

void parseConfigFile(const std::string &filePath, ServerConfig &config) {
    std::ifstream configFile(filePath.c_str());

    if (!configFile.is_open()) {
        throw std::runtime_error("Unable to open config file: " + filePath);
    }

    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key;

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        iss >> key;

        if (key == "listen") {
            int port;
            if (!(iss >> port)) {
                std::cerr << "Invalid port value" << std::endl;
                continue;
            }
            config.setPort(port);
        }
        else if (key == "server_name") {
            std::string serverName;
            if (!(iss >> serverName)) {
                std::cerr << "Invalid server name value" << std::endl;
                continue;
            }
            config.setServerName(serverName);
        }
        else if (key == "root") {
            std::string root;
            if (!(iss >> root)) {
                std::cerr << "Invalid root value" << std::endl;
                continue;
            }
            config.setRoot(root);
        }
        else if (key == "index") {
            std::string indexFile;
            if (!(iss >> indexFile)) {
                std::cerr << "Invalid index file value" << std::endl;
                continue;
            }
            config.setIndexFile(indexFile);
        }
        else if (key == "client_max_body_size") {
            std::string sizeStr;
            if (!(iss >> sizeStr)) {
                std::cerr << "Invalid client max body size value" << std::endl;
                continue;
            }

            // Convertir tamaño de "2M" a bytes, ejemplo simple
           size_t clientMaxBodySize = stringToInt(sizeStr.substr(0, sizeStr.size() - 1)) * 1024 * 1024;
            config.setClientMaxBodySize(clientMaxBodySize);
        }
        else if (key == "error_page") {
            int errorCode;
            std::string errorPage;
            if (!(iss >> errorCode >> errorPage)) {
                std::cerr << "Invalid error page format" << std::endl;
                continue;
            }
            std::string errorPages = config.getErrorPages();
            errorPages += toString(errorCode) + " " + errorPage + "; ";
            config.setErrorPages(errorPages);
        }
        else if (key == "location") {
            std::string locationPath;
            if (!(iss >> locationPath)) {
                std::cerr << "Invalid location path" << std::endl;
                continue;
            }
            if (locationPath == "/") {
                std::string param;
                while (iss >> param) {
                    if (param == "allowed_methods") {
                        std::string methods;
                        if (!(iss >> methods)) {
                            std::cerr << "Invalid methods value" << std::endl;
                            continue;
                        }
                        config.setAllowedMethods(methods);
                    }
                    else if (param == "autoindex") {
                        std::string autoindex;
                        if (!(iss >> autoindex)) {
                            std::cerr << "Invalid autoindex value" << std::endl;
                            continue;
                        }
                        config.setAutoindex(autoindex == "on");
                    }
                    else if (param == "upload_dir") {
                        std::string uploadDir;
                        if (!(iss >> uploadDir)) {
                            std::cerr << "Invalid upload dir value" << std::endl;
                            continue;
                        }
                        config.setUploadSavePath(uploadDir);
                    }
                }
            }
            else if (locationPath == "/cgi-bin/") {
                std::string param;
                while (iss >> param) {
                    if (param == "cgi_extension") {
                        std::string cgiExt;
                        if (!(iss >> cgiExt)) {
                            std::cerr << "Invalid CGI extension value" << std::endl;
                            continue;
                        }
                        config.setCgiExtension(cgiExt);
                    }
                    else if (param == "cgi_bin") {
                        std::string cgiBin;
                        if (!(iss >> cgiBin)) {
                            std::cerr << "Invalid CGI bin value" << std::endl;
                            continue;
                        }
                        config.setCgiBinPath(cgiBin);
                    }
                }
            }
        }
    }

    configFile.close();
}

