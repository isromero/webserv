/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 13:44:05 by isromero          #+#    #+#             */
/*   Updated: 2024/08/30 18:24:34 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request(int clientfd, const ServerConfig &config) : _request(""), _method(""), _path(""), _headers(), _body(""), _isChunked(false), _config(config)
{
	this->_readRequest(clientfd);
}

Request::~Request() {}

void Request::_readRequest(int clientfd)
{
	// Just read the data from the client
	char buffer[1024];
	ssize_t bytesRead;
	bool headersRead = false;
	size_t contentLength = 0;
	size_t totalBodyRead = 0;

	while ((bytesRead = recv(clientfd, buffer, sizeof(buffer) - 1, 0)) > 0)
	{
		buffer[bytesRead] = '\0';
		this->_request.append(buffer, bytesRead);

		if (!headersRead)
		{
			size_t pos = this->_request.find("\r\n\r\n");
			if (pos != std::string::npos)
			{
				headersRead = true;

				std::string headers = this->_request.substr(0, pos);
				size_t contentLengthPos = headers.find("Content-Length: ");
				if (contentLengthPos != std::string::npos)
				{
					contentLengthPos += 16; // "Content-Length: " is 16 characters long
					size_t endOfLength = headers.find("\r\n", contentLengthPos);
					if (endOfLength != std::string::npos)
						contentLength = std::atoi(headers.substr(contentLengthPos, endOfLength - contentLengthPos).c_str()); // Get the content length number
				}

				size_t chunkedPos = headers.find("Transfer-Encoding: chunked");
				if (chunkedPos != std::string::npos)
				{
					this->_isChunked = true;
					contentLength = 0; // Chunked encoding, we don't know the content length
				}

				// Calculate the total body read
				totalBodyRead = this->_request.size() - (pos + 4);
			}
		}
		else
			totalBodyRead += bytesRead;

		if (headersRead && totalBodyRead >= contentLength)
			break;

		if (this->_request.size() > this->_config.getClientMaxBodySize())
			break; // Request too large, we don't read more for preventing attacks, then in parsing we return 400
	}
	if (bytesRead == -1)
	{
		std::cerr << "Error: reading the data from the client: " << strerror(errno) << std::endl;
		close(clientfd);
	}
	else if (bytesRead == 0)
	{
		std::cout << "Client disconnected" << std::endl;
		close(clientfd);
		this->_request = "";
	}
}

// Parsing Request following RFC 9112
StatusCode Request::parseRequest()
{
	if (this->_request.empty())
		return ERROR_400;
	else if (this->_request.size() >= this->_config.getClientMaxBodySize())
		return ERROR_413;

	size_t pos = 0;
	size_t end = 0;
	size_t contentLength = 0;

	StatusCode statusCode = NO_STATUS_CODE;

	statusCode = this->_parseRequestLine(pos, end);
	if (statusCode != NO_STATUS_CODE)
		return statusCode;

	statusCode = this->_parseHeaders(pos, end, contentLength);
	if (statusCode != NO_STATUS_CODE)
		return statusCode;

	statusCode = this->_parseBody(pos, contentLength);
	if (statusCode != NO_STATUS_CODE)
		return statusCode;

	return NO_STATUS_CODE;
}

StatusCode Request::_parseRequestLine(size_t &pos, size_t &end)
{
	// Leading empty lines prior to the request-line
	while (pos < this->_request.size() && (this->_request[pos] == '\r' || this->_request[pos] == '\n'))
	{
		if (this->_request[pos] == '\r' && pos + 1 < this->_request.size() && this->_request[pos + 1] == '\n')
			pos += 2;
		else
			pos++;
	}

	// Parse the request-line
	end = this->_request.find("\r\n", pos);
	if (end == std::string::npos)
	{
		end = this->_request.find('\n', pos);
		if (end == std::string::npos)
			return ERROR_400;
	}

	std::string requestLine = this->_request.substr(pos, end - pos);
	size_t methodEnd = requestLine.find(' ');
	size_t fileStart = requestLine.find('/', methodEnd);
	size_t fileEnd = requestLine.find(' ', fileStart + 1);
	size_t versionStart = requestLine.find("HTTP/", fileEnd);

	if (methodEnd != std::string::npos && fileStart != std::string::npos && fileEnd != std::string::npos)
	{
		this->_method = requestLine.substr(0, methodEnd);
		if (this->_method != "GET" && this->_method != "POST" && this->_method != "DELETE") // Only GET, POST and DELETE are supported
			return ERROR_405;

		std::string version = requestLine.substr(versionStart);
		if (version != "HTTP/1.1")
			return ERROR_505;

		std::string uri = requestLine.substr(fileStart, fileEnd - fileStart);
		if (uri.size() > 2048)
			return ERROR_414;
		if (uri.find("://") != std::string::npos)
		{
			// Absolute-form request, extract path after the authority. Eg: GET http://localhost:6969/index.html HTTP/1.1
			size_t pathStart = uri.find('/', uri.find("://") + 3);
			if (pathStart != std::string::npos)
				this->_path = uri.substr(pathStart);
			else
				this->_path = "/";
		}
		else
			this->_path = uri; // Relative-form this->_request. Eg: GET /index.html HTTP/1.1
		if (this->_path.find(' ') != std::string::npos || this->_path.empty())
			return ERROR_400;
		this->_path = secureFilePath(this->_path);

		if (!this->_config.isMethodAllowed(this->_config.getLocations(), this->_path, this->_method))
			return ERROR_405;
	}
	else if (requestLine.find("HTTP/1.1") == std::string::npos)
		return ERROR_505;
	else
		return ERROR_400;

	pos = end + 1;
	if (this->_request[pos] == '\n')
		pos++; // Skip the \n character if we encountered \r\n

	// Check for whitespace between request-line and first header field
	if (pos < this->_request.size() && (this->_request[pos] == ' ' || this->_request[pos] == '\t'))
	{
		// Whitespace detected, consume whitespace-preceded lines
		while (pos < this->_request.size())
		{
			end = this->_request.find("\r\n", pos);
			if (end == std::string::npos)
			{
				end = this->_request.find('\n', pos);
				if (end == std::string::npos)
					break; // End of request
			}

			// Check if this line starts with a valid header field
			if (this->_request[pos] != ' ' && this->_request[pos] != '\t')
			{
				size_t colon = this->_request.find(':', pos);
				if (colon != std::string::npos && colon < end && colon > pos)
					break; // Valid header field found, stop consuming
			}

			// Consume this line
			pos = end + 1;
			if (this->_request[pos] == '\n')
				pos++;
		}
	}

	return NO_STATUS_CODE;
}

StatusCode Request::_parseHeaders(size_t &pos, size_t &end, size_t &contentLength)
{
	// Parse headers
	while (pos < this->_request.size())
	{
		end = this->_request.find("\r\n", pos);
		if (end == std::string::npos)
		{
			end = this->_request.find('\n', pos);
			if (end == std::string::npos)
				break; // End of request
		}

		if (end == pos || (end == pos + 1 && this->_request[pos] == '\r'))
		{
			// Empty line, end of headers
			pos = end + 1;
			if (this->_request[pos] == '\n')
				pos++;
			break;
		}

		std::string headerLine = this->_request.substr(pos, end - pos);
		size_t separator = headerLine.find(": ");
		if (separator != std::string::npos)
		{
			std::string headerName = headerLine.substr(0, separator);
			std::string headerValue = headerLine.substr(separator + 2);

			// Replace bare CR with SP in header value
			for (size_t i = 0; i < headerValue.size(); ++i)
			{
				if (headerValue[i] == '\r' && (i + 1 == headerValue.size() || headerValue[i + 1] != '\n'))
					headerValue[i] = ' ';
			}

			this->_headers[headerName] = headerValue;
		}
		else
			return ERROR_400;

		pos = end + 1;
		if (this->_request[pos] == '\n')
			pos++;
	}

	/* HOST CHECK */
	// Check for Host header is only one(it is necessary just one)
	size_t hostCount = this->_headers.count("Host");
	if (hostCount != 1)
		return ERROR_400;

	// Check for Host header value
	std::map<std::string, std::string>::iterator hostIt = this->_headers.find("Host");
	if (hostIt == this->_headers.end() || hostIt->second.empty()) // Header Host not found or empty value of Host
		return ERROR_400;

	std::string hostValue = hostIt->second;
	std::vector<std::string> serverNames = this->_config.getServerNames();
	bool hasPort = false;

	// Remove port from hostValue if present for comparison
	size_t colonPos = hostValue.find(':');
	if (colonPos != std::string::npos)
		hasPort = true;
	std::string hostWithoutPort = hostValue.substr(0, colonPos);

	bool hostMatches = false;
	for (std::vector<std::string>::iterator it = serverNames.begin(); it != serverNames.end(); ++it)
	{
		std::string serverNameWithPort = *it + ":" + toString(this->_config.getPort());
		if (serverNameWithPort == hostValue || (*it == hostWithoutPort && !hasPort))
		{
			hostMatches = true;
			break;
		}
	}

	if (!hostMatches)
		return ERROR_400;

	/* CONTENT-LENGTH CHECK */
	std::map<std::string, std::string>::iterator it = this->_headers.find("Content-Length");
	if (it != this->_headers.end())
		contentLength = static_cast<size_t>(std::atoi(it->second.c_str()));
	else
	{
		// No Content-Length specified, check for Transfer-Encoding
		it = this->_headers.find("Transfer-Encoding");
		if (it != this->_headers.end() && it->second == "chunked")
			return ERROR_411;
	}

	return NO_STATUS_CODE;
}

StatusCode Request::_parseBody(size_t &pos, size_t &contentLength)
{
	if (this->_isChunked)
	{
		// When a request is chunked looks like this: POST /index.html HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n7\r\nchunked\r\n0\r\n\r\n
		// The numbers are the size of the chunk in hexadecimal, so we need to convert them to integer
		std::string dechunkedBody;
		while (1)
		{
			size_t lineEnd = this->_request.find("\r\n", pos);
			if (lineEnd == std::string::npos)
				return ERROR_400;

			std::string chunkSizeStr = this->_request.substr(pos, lineEnd - pos);
			size_t chunkSize = std::strtol(chunkSizeStr.c_str(), NULL, 16); // Convert the chunk size from hexadecimal to integer

			pos = lineEnd + 2; // Skip the \r\n

			if (chunkSize == 0) // Last chunk
				break;

			if (pos + chunkSize > this->_request.size())
				return ERROR_400;

			dechunkedBody += this->_request.substr(pos, chunkSize);
			pos += chunkSize + 2; // Skip the \r\n
		}
		this->_body = dechunkedBody;

		return NO_STATUS_CODE;
	}

	size_t remainingLength = this->_request.size() - pos;

	if (contentLength > 0)
	{
		if (remainingLength < contentLength)
			return ERROR_400;
		this->_body = this->_request.substr(pos, contentLength); // Truncate the body to the specified Content-Length
	}
	else if (remainingLength > 0)
		this->_body = this->_request.substr(pos); // No Content-Length specified, consume the remaining data

	return NO_STATUS_CODE;
}

const std::string &Request::getRequest() const
{
	return this->_request;
}

const std::string &Request::getMethod() const
{
	return this->_method;
}

const std::string &Request::getPath() const
{
	return this->_path;
}

const std::map<std::string, std::string> &Request::getHeaders() const
{
	return this->_headers;
}

const std::string &Request::getBody() const
{
	return this->_body;
}
