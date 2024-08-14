/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:54:49 by isromero          #+#    #+#             */
/*   Updated: 2024/08/14 17:18:48 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response(const std::string &request, const std::string &method, const std::string &requestedFile, const std::map<std::string, std::string> &headers, const std::string &body)
	: _response(""), _request(request), _method(method), _requestedFile(requestedFile), _headers(headers), _body(body)
{
}

Response::~Response()
{
}

const std::string &Response::handleMethods()
{
	if (this->_method == "GET")
		this->_handleGET();
	else if (this->_method == "POST")
		this->_handlePOST();
	return this->_response;
}

void Response::_handlePOST()
{
	// TODO: Refactor using variables of the class + fix some things of post
	std::string status = "400 Bad Request";
	std::string responseBody = "400 Bad Request: Malformed POST request.";
	std::string contentType;
	std::string body;

	size_t headerEnd = this->_request.find("\r\n\r\n");
	if (headerEnd != std::string::npos)
	{
		body = this->_request.substr(headerEnd + 4);

		size_t contentTypeStart = this->_request.find("Content-Type: ");
		if (contentTypeStart != std::string::npos)
		{
			size_t contentTypeEnd = this->_request.find("\r\n", contentTypeStart);
			contentType = this->_request.substr(contentTypeStart + 14, contentTypeEnd - contentTypeStart - 14);
		}
	}

	if (body.empty())
	{
		if (contentType.empty())
		{
			status = "400 Bad Request";
			responseBody = "400 Bad Request: Content-Type header missing.";
		}
		else
		{
			status = "400 Bad Request";
			responseBody = "400 Bad Request: The POST request body cannot be empty.";
		}
	}
	else if (contentType == "application/json")
	{
		if (body.size() >= 2 && body[0] == '{' && body[body.size() - 1] == '}')
		{
			status = "201 Created";
			responseBody = "201 Created: Data received and processed successfully.";
		}
		else
		{
			status = "400 Bad Request";
			responseBody = "400 Bad Request: Malformed JSON body.";
		}
	}
	else if (contentType == "application/x-www-form-urlencoded")
	{
		if (body.find('=') != std::string::npos && body.find('&') != std::string::npos)
		{
			status = "201 Created";
			responseBody = "201 Created: Data received and processed successfully.";
		}
		else
		{
			status = "400 Bad Request";
			responseBody = "400 Bad Request: Malformed application/x-www-form-urlencoded body.";
		}
	}
	else if (contentType == "multipart/form-data")
	{
		size_t boundaryStart = this->_request.find("boundary=");
		if (boundaryStart != std::string::npos)
		{
			std::string boundary = this->_request.substr(boundaryStart + 9);
			if (body.find(boundary) != std::string::npos)
			{
				status = "415 Unsupported Media Type";
				responseBody = "415 Unsupported Media Type: The server cannot handle this content type.";
			}
			else
			{
				status = "400 Bad Request";
				responseBody = "400 Bad Request: Malformed multipart/form-data body.";
			}
		}
		else
		{
			status = "400 Bad Request";
			responseBody = "400 Bad Request: Malformed multipart/form-data. Boundary missing.";
		}
	}
	else if (contentType == "text/plain")
	{
		status = "201 Created";
		responseBody = "201 Created: Data received and processed successfully.";
	}
	else
	{
		status = "415 Unsupported Media Type";
		responseBody = "415 Unsupported Media Type: The server cannot handle this content type.";
	}

	this->_response = "HTTP/1.1 " + status + "\r\n" +
					  "Content-Type: text/plain\r\n" +
					  "Content-Length: " + toString(responseBody.size()) + "\r\n" +
					  "Connection: close\r\n" + // TODO: Check if we need to close the connection / put in the headers??
					  "\r\n" +
					  responseBody;
}

void Response::_handleGET()
{
	// TODO: Refactor and check the requested file here?????
	// TODO: Refactor and create enums, generation of error codes like requests differently...???
	std::string file;
	std::string status;

	if (this->_requestedFile[this->_requestedFile.size() - 1] == '/') // If is a directory just serve the index.html or index.htm
	{
		if (access(("pages" + this->_requestedFile + "index.html").c_str(), F_OK) == 0)
			file = "pages" + this->_requestedFile + "index.html";
		else if (access(("pages" + this->_requestedFile + "index.htm").c_str(), F_OK) == 0)
			file = "pages" + this->_requestedFile + "index.htm";
	}
	else
		file = "pages" + this->_requestedFile;

	// Check if the file has an extension, if not add .html
	if (file.find_last_of(".") == std::string::npos)
		file += ".html";

	// Check if the file exists
	if (access(file.c_str(), F_OK) != 0)
	{
		status = "404 Not Found";
		file = "pages/404.html";
	}
	else
	{
		// Check if the file is readable
		if (access(file.c_str(), R_OK) != 0)
		{
			status = "403 Forbidden";
			file = "pages/403.html";
		}
		else
			status = "200 OK";
	}

	std::string fileContent = readFile(file);
	const std::string contentType = this->_determineContentType(file);
	if (status.empty())
		status = "200 OK";
	this->_response = "HTTP/1.1 " + status + "\r\n" +
					  "Content-Type: " + contentType + "\r\n" +
					  "Content-Length: " + toString(fileContent.size()) + "\r\n\r\n" + fileContent;
}

const std::string Response::_determineContentType(const std::string &filename)
{
	if (filename.find(".html") != std::string::npos || filename.find(".htm") != std::string::npos)
		return "text/html";
	else if (filename.find(".css") != std::string::npos)
		return "text/css";
	else if (filename.find(".js") != std::string::npos)
		return "application/javascript";
	else if (filename.find(".json") != std::string::npos)
		return "application/json";
	else if (filename.find(".xml") != std::string::npos)
		return "application/xml";
	else if (filename.find(".pdf") != std::string::npos)
		return "application/pdf";
	else if (filename.find(".jpg") != std::string::npos || filename.find(".jpeg") != std::string::npos)
		return "image/jpeg";
	else if (filename.find(".png") != std::string::npos)
		return "image/png";
	else if (filename.find(".gif") != std::string::npos)
		return "image/gif";
	else if (filename.find(".svg") != std::string::npos)
		return "image/svg+xml";
	else if (filename.find(".ico") != std::string::npos)
		return "image/x-icon";
	else if (filename.find(".tif") != std::string::npos || filename.find(".tiff") != std::string::npos)
		return "image/tiff";
	else if (filename.find(".webp") != std::string::npos)
		return "image/webp";
	else if (filename.find(".mp3") != std::string::npos)
		return "audio/mpeg";
	else if (filename.find(".wav") != std::string::npos)
		return "audio/wav";
	else if (filename.find(".mp4") != std::string::npos)
		return "video/mp4";
	else if (filename.find(".avi") != std::string::npos)
		return "video/x-msvideo";
	else if (filename.find(".mpeg") != std::string::npos || filename.find(".mpg") != std::string::npos)
		return "video/mpeg";
	else if (filename.find(".webm") != std::string::npos)
		return "video/webm";
	else if (filename.find(".zip") != std::string::npos)
		return "application/zip";
	else if (filename.find(".tar") != std::string::npos)
		return "application/x-tar";
	else if (filename.find(".gz") != std::string::npos || filename.find(".gzip") != std::string::npos)
		return "application/gzip";
	else if (filename.find(".txt") != std::string::npos)
		return "text/plain";
	else if (filename.find(".rtf") != std::string::npos)
		return "application/rtf";
	else if (filename.find(".doc") != std::string::npos || filename.find(".docx") != std::string::npos)
		return "application/msword";
	else if (filename.find(".xls") != std::string::npos || filename.find(".xlsx") != std::string::npos)
		return "application/vnd.ms-excel";
	else if (filename.find(".ppt") != std::string::npos || filename.find(".pptx") != std::string::npos)
		return "application/vnd.ms-powerpoint";
	else
		return "application/octet-stream"; // Default binary file type
}

const std::string &Response::getResponse() const
{
	return this->_response;
}
