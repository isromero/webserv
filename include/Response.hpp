/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:52:54 by isromero          #+#    #+#             */
/*   Updated: 2024/08/28 19:02:21 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "includes.hpp"
#include "ServerConfig.hpp"
#include "status.hpp"
#include "utils.hpp"

#if defined(__APPLE__)
extern char **environ;
#endif

class Response
{
private:
	std::string _response;
	std::string _request;
	std::string _method;
	std::string _requestedPath;
	std::map<std::string, std::string> _requestHeaders;
	std::string _requestBody;
	std::map<std::string, std::string> _responseHeaders;
	std::string _responseBody;
	std::string _responsePath;
	std::string _locationHeader;
	std::map<std::string, std::string> _cgiHeaders;
	std::string _cgiBody;
	ServerConfig _config;

	StatusCode _handleCGI();
	StatusCode _handleGET();
	StatusCode _handlePOST();
	StatusCode _handleDELETE();

	bool _isCGIRequest() const;
	const std::string _determineContentType(const std::string &filename) const;
	const std::string _generateHTMLPage(bool isError, const std::string &statusLine, const std::string &body);
	const std::string _generateDirectoryListing();
	bool _saveFile(const std::string &content, const std::string &filename, std::string &savedPath);
	bool _hasIndexFileInResponse() const;

public:
	Response(const std::string &request, const std::string &method, const std::string &requestedPath, const std::map<std::string, std::string> &headers, const std::string &body, const ServerConfig &config);
	~Response();

	const std::string handleResponse(StatusCode statusCode);
	StatusCode handleMethods();
};

#endif
