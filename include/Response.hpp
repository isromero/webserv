/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:52:54 by isromero          #+#    #+#             */
/*   Updated: 2024/08/15 11:21:21 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <string>
#include <map>
#include <unistd.h>

#include "status.hpp"
#include "utils.hpp"

class Response
{
private:
	std::string _response;
	std::string _request;
	std::string _method;
	std::string _requestedFile;
	std::map<std::string, std::string> _headers; // TODO: This is not headers, this is requestHeaders, and then create another for the response
	std::string _body;							 // TODO: This is not body, this is requestHeaders, and then create another for the response

	StatusCode _handleGET();
	StatusCode _handlePOST();
	StatusCode _handleDELETE();

	const std::string _determineContentType(const std::string &filename);

public:
	Response(const std::string &request, const std::string &method, const std::string &requestedFile, const std::map<std::string, std::string> &headers, const std::string &body);
	~Response();

	const std::string handleResponse(StatusCode statusCode);
	StatusCode &handleMethods();
};

#endif
