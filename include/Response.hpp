/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adgutier <adgutier@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:52:54 by isromero          #+#    #+#             */
/*   Updated: 2024/08/14 19:59:24 by adgutier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <string>
#include <map>
#include <unistd.h>

#include "errors.hpp"
#include "utils.hpp"

class Response
{
private:
	std::string _response;
	std::string _request;
	std::string _method;
	std::string _requestedFile;
	std::map<std::string, std::string> _headers;
	std::string _body;

	void _handleGET();
	void _handlePOST();
	void _handleDELETE();

	const std::string _determineContentType(const std::string &filename);

public:
	Response(const std::string &request, const std::string &method, const std::string &requestedFile, const std::map<std::string, std::string> &headers, const std::string &body);
	~Response();

	const std::string &handleMethods();

	const std::string &getResponse() const;
};

#endif
