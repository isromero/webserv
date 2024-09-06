/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 13:42:12 by isromero          #+#    #+#             */
/*   Updated: 2024/08/30 18:24:36 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "includes.hpp"
#include "ServerConfig.hpp"
#include "status.hpp"
#include "utils.hpp"

class Request
{
private:
	std::string _request;
	std::string _method;
	std::string _path;
	std::map<std::string, std::string> _headers;
	std::string _body;
	bool _isChunked;
	ServerConfig _config;

	void _readRequest(int clientfd);

	StatusCode _parseRequestLine(size_t &pos, size_t &end);
	StatusCode _parseHeaders(size_t &pos, size_t &end, size_t &contentLength);
	StatusCode _parseBody(size_t &pos, size_t &contentLength);

public:
	Request(int clientfd, const ServerConfig &config);
	~Request();

	StatusCode parseRequest();

	const std::string &getRequest() const;
	const std::string &getMethod() const;
	const std::string &getPath() const;
	const std::map<std::string, std::string> &getHeaders() const;
	const std::string &getBody() const;
};

#endif
