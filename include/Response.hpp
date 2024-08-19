/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:52:54 by isromero          #+#    #+#             */
/*   Updated: 2024/08/19 19:22:25 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <string>
#include <map>
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>

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
	std::string _requestedFile;
	std::map<std::string, std::string> _requestHeaders;
	std::string _requestBody;
	std::map<std::string, std::string> _responseHeaders;
	std::string _responseBody;
	std::string _responseFile;
	std::string _locationHeader;
	std::map<std::string, std::string> _cgiHeaders;
	std::string _cgiBody;

	StatusCode _handleCGI();
	StatusCode _handleGET();
	StatusCode _handlePOST();
	StatusCode _handleDELETE();

	bool _isCGIRequest();
	const std::string _determineContentType(const std::string &filename);
	const std::string _generateHTMLPage(bool isError, const std::string &statusLine, const std::string &body);

public:
	Response(const std::string &request, const std::string &method, const std::string &requestedFile, const std::map<std::string, std::string> &headers, const std::string &body);
	~Response();

	const std::string handleResponse(StatusCode statusCode);
	StatusCode handleMethods();
};

#endif
