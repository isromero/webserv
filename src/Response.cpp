/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:54:49 by isromero          #+#    #+#             */
/*   Updated: 2024/08/18 17:08:59 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response(const std::string &request, const std::string &method, const std::string &requestedFile, const std::map<std::string, std::string> &headers, const std::string &body)
	: _response(""), _request(request), _method(method), _requestedFile(requestedFile), _requestHeaders(headers), _requestBody(body), _responseHeaders(), _responseBody(""), _responseFile(""), _locationHeader("")
{
}

Response::~Response()
{
}

const std::string Response::handleResponse(StatusCode statusCode)
{
	std::string statusLine;
	bool isError = false;

	switch (statusCode)
	{
	case SUCCESS_200:
		statusLine = "HTTP/1.1 200 OK";
		this->_responseBody = readFile(this->_responseFile);
		break;
	case SUCCESS_201:
		statusLine = "HTTP/1.1 201 Created";
		this->_responseBody = "The request was successful and a new resource was created.";
		break;
	case SUCCESS_204:
		statusLine = "HTTP/1.1 204 No Content"; // No body if no content
		break;
	case ERROR_400:
		statusLine = "HTTP/1.1 400 Bad Request";
		this->_responseBody = "The server cannot process the request due to a client error.";
		isError = true;
		break;
	case ERROR_403:
		statusLine = "HTTP/1.1 403 Forbidden";
		this->_responseBody = "The server understood the request, but is refusing to fulfill it.";
		isError = true;
		break;
	case ERROR_404:
		statusLine = "HTTP/1.1 404 Not Found";
		this->_responseBody = "The server has not found anything matching the request URI.";
		isError = true;
		break;
	case ERROR_405:
		statusLine = "HTTP/1.1 405 Method Not Allowed";
		this->_responseBody = "405 Method Not Allowed: The method specified in the request is not allowed.";
		isError = true;
		break;
	case ERROR_411:
		statusLine = "HTTP/1.1 411 Length Required";
		this->_responseBody = "The request did not specify the length of its content.";
		isError = true;
		break;
	case ERROR_413:
		statusLine = "HTTP/1.1 413 Payload Too Large";
		this->_responseBody = "The request is larger than the server is willing or able to process.";
		isError = true;
		break;
	case ERROR_414:
		statusLine = "HTTP/1.1 414 URI Too Long";
		this->_responseBody = "The URI provided was too long for the server to process.";
		isError = true;
		break;
	case ERROR_415:
		statusLine = "HTTP/1.1 415 Unsupported Media Type";
		this->_responseBody = "The server cannot handle this content type.";
		isError = true;
		break;
	case ERROR_500:
		statusLine = "HTTP/1.1 500 Internal Server Error";
		this->_responseBody = "The server encountered an unexpected condition that prevented it from fulfilling the request.";
		isError = true;
		break;
	case ERROR_505:
		statusLine = "HTTP/1.1 505 HTTP Version Not Supported";
		this->_responseBody = "The HTTP version used in the request is not supported by the server.";
		isError = true;
		break;
	default:
		statusLine = "HTTP/1.1 500 Internal Server Error";
		this->_responseBody = "The server encountered an unexpected condition that prevented it from fulfilling the request.";
		isError = true;
		break;
	}

	// Build the response

	// if isError is true, the content type will be text/html because we return an error message
	if (isError)
	{
		this->_responseHeaders["Content-Type"] = "text/html"; // Error pages are always html
		this->_responseBody = this->_generateHTMLPage(isError, statusLine, this->_responseBody);
	}
	else if (!isError)
	{
		if (statusCode == SUCCESS_200) // If it is a success, we determine the content type because we are serving a file
			this->_responseHeaders["Content-Type"] = this->_determineContentType(this->_responseFile);
		else
		{
			this->_responseHeaders["Content-Type"] = "text/html"; // Other sucess messages we return an HTML page
			this->_responseBody = this->_generateHTMLPage(isError, statusLine, this->_responseBody);
		}
	}

	this->_responseHeaders["Content-Length"] = toString(this->_responseBody.size());

	if (this->_locationHeader != "")
		this->_responseHeaders["Location"] = this->_locationHeader;

	this->_response = statusLine + "\r\n";
	for (std::map<std::string, std::string>::iterator it = this->_responseHeaders.begin(); it != this->_responseHeaders.end(); ++it)
		this->_response += it->first + ": " + it->second + "\r\n"; // Add all headers to response
	this->_response += "\r\n";
	this->_response += this->_responseBody;

	return this->_response;
}

StatusCode Response::handleMethods()
{
	if (this->_method == "GET")
		return this->_handleGET();
	else if (this->_method == "POST")
		return this->_handlePOST();
	else if (this->_method == "DELETE")
		return this->_handleDELETE();
	return NO_STATUS_CODE; // Impossible to reach this point
}

StatusCode Response::_handleGET()
{
	if (this->_requestedFile[this->_requestedFile.size() - 1] == '/') // If is a directory just serve the index.html or index.htm
	{
		if (access(("pages" + this->_requestedFile + "index.html").c_str(), F_OK) == 0)
			this->_responseFile = "pages" + this->_requestedFile + "index.html";
		else if (access(("pages" + this->_requestedFile + "index.htm").c_str(), F_OK) == 0)
			this->_responseFile = "pages" + this->_requestedFile + "index.htm";
	}
	else
		this->_responseFile = "pages" + this->_requestedFile;

	// Check if the file has an extension, if not add .html
	if (this->_responseFile.find_last_of(".") == std::string::npos)
		this->_responseFile += ".html";

	// Check if the file exists
	if (access(this->_responseFile.c_str(), F_OK) != 0)
		return ERROR_404;
	else
	{
		// Check if the file is readable
		if (access(this->_responseFile.c_str(), R_OK) != 0)
			return ERROR_403;
		else
			return SUCCESS_200;
	}

	return NO_STATUS_CODE; // Impossible to reach this point
}

StatusCode Response::_handlePOST()
{
	std::string contentType = this->_requestHeaders["Content-Type"];

	if (contentType.find("multipart/form-data") != std::string::npos) // Handle file uploads
	{
		// Boundary is something like "----WebKitFormBoundary7MA4YWxkTrZu0gW" and it is in the Content-Type header
		// In the body, the boundary is preceded by "--" and followed by "\r\n" and each part is separated by "--" + boundary
		size_t boundaryPos = contentType.find("boundary=");
		if (boundaryPos == std::string::npos)
			return ERROR_400;

		std::string boundary = contentType.substr(boundaryPos + 9);
		std::string delimiter = "--" + boundary;

		size_t pos = 0;
		bool fileUploaded = false;
		while ((pos = this->_requestBody.find(delimiter, pos)) != std::string::npos)
		{
			size_t nextPos = this->_requestBody.find(delimiter, pos + delimiter.size());
			if (nextPos == std::string::npos)
				break;

			std::string part = this->_requestBody.substr(pos + delimiter.size(), nextPos - pos - delimiter.size());

			size_t filenamePos = part.find("filename=\"");
			if (filenamePos != std::string::npos)
			{
				size_t filenameEnd = part.find("\"", filenamePos + 10);
				if (filenameEnd != std::string::npos)
				{
					std::string filename = part.substr(filenamePos + 10, filenameEnd - filenamePos - 10);
					size_t contentStart = part.find("\r\n\r\n");
					if (contentStart != std::string::npos)
					{
						std::string content = part.substr(contentStart + 4);
						std::string savedPath;
						if (saveFile(content, filename, savedPath))
						{
							this->_locationHeader = savedPath;
							fileUploaded = true;
						}
						else
							return ERROR_500;
					}
				}
			}
			pos = nextPos;
		}

		return fileUploaded ? SUCCESS_201 : SUCCESS_200;
	}
	else if (contentType.find("application/x-www-form-urlencoded") != std::string::npos) // Form data, this is the typical "key=value" format
	{
		// Check for valid key=value pairs
		size_t pos = 0;
		while (pos < this->_requestBody.size())
		{
			size_t eqPos = this->_requestBody.find('=', pos);
			if (eqPos == std::string::npos || eqPos == pos)
				return ERROR_400;

			size_t ampPos = this->_requestBody.find('&', eqPos);
			if (ampPos == std::string::npos)
				ampPos = this->_requestBody.size();

			if (ampPos <= eqPos + 1)
				return ERROR_400;

			pos = ampPos + 1;
		}
		return SUCCESS_200; // Valid but we don't do anything with it(not saving in the server)
	}
	else if (contentType.find("application/json") != std::string::npos) // JSON data
	{

		if (this->_requestBody.size() >= 2 && this->_requestBody[0] == '{' && this->_requestBody[this->_requestBody.size() - 1] == '}')
			return SUCCESS_200; // JSON is valid, but we don't do anything with it(not saving in the server)
		else
			return ERROR_400;
	}
	else if (contentType.find("text/html") != std::string::npos)
	{
		// Basic HTML structure check
		if (this->_requestBody.find("<html") != std::string::npos &&
			this->_requestBody.find("</html>") != std::string::npos)
			return SUCCESS_200; // Valid but we don't do anything with it(not saving in the server)
		else
			return ERROR_400;
	}
	else if (contentType.find("text/plain") != std::string::npos)
	{
		if (!this->_requestBody.empty())
			return SUCCESS_200; // Valid but we don't do anything with it(not saving in the server)
		else
			return ERROR_400;
	}
	else
		return ERROR_415;

	return NO_STATUS_CODE; // Impossible to reach this point
}

StatusCode Response::_handleDELETE()
{
	std::string file;
	if (this->_requestedFile.empty())
		return ERROR_400;
	else if (this->_requestHeaders["Content-Length"] != "0") // Body have to be empty
		return ERROR_400;
	else if (this->_requestedFile[this->_requestedFile.size() - 1] == '/') // If is a directory, we don't allow to delete it
		return ERROR_403;
	else
	{
		file = "pages" + this->_requestedFile;

		if (remove(file.c_str()) == 0)
			return SUCCESS_204;
		else
		{
			if (errno == ENOENT) // File not found
				return ERROR_404;
			else if (errno == EACCES) // Permission denied
				return ERROR_403;
			else
				return ERROR_500;
		}
	}

	return NO_STATUS_CODE; // Impossible to reach this point
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

const std::string Response::_generateHTMLPage(bool isError, const std::string &statusLine, const std::string &body)
{
	std::string responseBody;
	std::string status;

	status = statusLine.substr(9);

	responseBody += "<!DOCTYPE html>\n";
	responseBody += "<html lang=\"en\">\n";
	responseBody += "<head>\n";
	responseBody += "<meta charset=\"UTF-8\">\n";
	responseBody += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
	responseBody += "<title>" + status + "</title>\n"; // Remove "HTTP/1.1 "
	responseBody += "<style>\n";
	responseBody += "body { text-align: center; margin-top: 50px; }\n";
	if (isError)
		responseBody += "h1 { font-size: 50px; color: red; }\n";
	else
		responseBody += "h1 { font-size: 50px; color: green; }\n";
	responseBody += "p { font-size: 20px; }\n";
	responseBody += "</style>\n";
	responseBody += "</head>\n";
	responseBody += "<body>\n";
	responseBody += "<h1>" + status + "</h1>\n";
	responseBody += "<p>" + body + "</p>\n";
	responseBody += "</body>\n";
	responseBody += "</html>\n";

	return responseBody;
}
