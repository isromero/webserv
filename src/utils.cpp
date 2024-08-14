/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:29:36 by isromero          #+#    #+#             */
/*   Updated: 2024/08/14 16:41:31 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

const std::string generateStatusError(StatusErrorCode error)
{
	std::string statusLine, body;

	switch (error)
	{
	case INVALID_REQUEST:
	case INVALID_REQUEST_LINE:
	case INVALID_HEADER_FORMAT:
	case INCOMPLETE_BODY:
	case INVALID_REQUEST_TARGET:
		statusLine = "HTTP/1.1 400 Bad Request";
		body = "400 Bad Request: The server cannot process the request due to a client error.";
		break;
	case INVALID_METHOD:
		statusLine = "HTTP/1.1 405 Method Not Allowed";
		body = "405 Method Not Allowed: The method specified in the request is not allowed.";
		break;
	case INVALID_CONTENT_LENGTH:
		statusLine = "HTTP/1.1 411 Length Required";
		body = "411 Length Required: The request did not specify the length of its content.";
		break;
	case PAYLOAD_TOO_LARGE:
		statusLine = "HTTP/1.1 413 Payload Too Large";
		body = "413 Payload Too Large: The request is larger than the server is willing or able to process.";
		break;
	case URI_TOO_LONG:
		statusLine = "HTTP/1.1 414 URI Too Long";
		body = "414 URI Too Long: The URI provided was too long for the server to process.";
		break;
	case VERSION_NOT_SUPPORTED:
		statusLine = "HTTP/1.1 505 HTTP Version Not Supported";
		body = "505 HTTP Version Not Supported: The HTTP version used in the request is not supported by the server.";
		break;
	default:
		statusLine = "HTTP/1.1 500 Internal Server Error";
		body = "500 Internal Server Error: The server encountered an unexpected condition that prevented it from fulfilling the request.";
		break;
	}

	std::string response = statusLine + "\r\n";
	response += "Content-Type: text/plain\r\n"; // TODO: Create function to create error pages with html and don't return text/plain???

	std::stringstream ss;
	ss << body.size();
	response += "Content-Length: " + ss.str() + "\r\n";

	response += "\r\n";
	response += body;

	return response;
}

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
	std::ifstream file(filename.c_str());
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
