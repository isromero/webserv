/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:54:49 by isromero          #+#    #+#             */
/*   Updated: 2024/09/02 21:37:20 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response() : _response(""), _request(""), _method(""), _requestedPath(""), _requestHeaders(), _requestBody(""), _responseHeaders(), _responseBody(""), _responsePath(""), _locationHeader(""), _cgiHeaders(), _cgiBody(""), _config()
{
}

Response::Response(const std::string &request, const std::string &method, const std::string &requestedPath, const std::map<std::string, std::string> &headers, const std::string &body, const ServerConfig &config)
	: _response(""), _request(request), _method(method), _requestedPath(requestedPath), _requestHeaders(headers), _requestBody(body), _responseHeaders(), _responseBody(""), _responsePath(""), _locationHeader(""), _cgiHeaders(), _cgiBody(""), _config(config)
{
}

Response::~Response()
{
}

bool Response::_saveFile(const std::string &content, const std::string &filename, std::string &savedPath)
{
	std::string uploadDir = this->_config.getUploadDir(this->_requestedPath);
	if (uploadDir[uploadDir.size() - 1] != '/')
		uploadDir += "/";
	std::string filepath = uploadDir + getFilenameAndDate(filename);
	std::ofstream file(filepath.c_str(), std::ios::binary);
	if (file.is_open())
	{
		file.write(content.c_str(), content.size());
		file.close();
		savedPath = filepath;
		return true;
	}
	return false;
}

const std::string Response::handleResponse(StatusCode statusCode)
{
	std::string statusLine;
	bool isError = false;

	switch (statusCode)
	{
	case SUCCESS_200:
		statusLine = "HTTP/1.1 200 OK";
		if (this->_method == "GET" && this->_hasIndexFileInResponse())
			this->_responseBody = readFile(this->_responsePath);
		break;
	case SUCCESS_201:
		statusLine = "HTTP/1.1 201 Created";
		this->_responseBody = "The request was successful and a new resource was created.";
		break;
	case SUCCESS_204:
		statusLine = "HTTP/1.1 204 No Content"; // No body if no content
		break;
	case REDIRECTION_301:
		statusLine = "HTTP/1.1 301 Moved Permanently";
		this->_responseBody = "The requested resource has been assigned a new permanent URI.";
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

	if (this->_isCGIRequest())
	{
		// Build the headers if CGI returned any
		size_t headerEnd = this->_cgiBody.find("\r\n\r\n");
		if (headerEnd == std::string::npos)
			headerEnd = this->_cgiBody.find("\n\n"); // Maybe the CGI script uses \n instead of \r\n
		if (headerEnd != std::string::npos)
		{
			std::string headers = this->_cgiBody.substr(0, headerEnd);
			this->_responseBody = this->_cgiBody.substr(headerEnd + (this->_cgiBody[headerEnd + 1] == '\n' ? 2 : 1));

			std::istringstream headerStream(headers);
			std::string header;
			while (std::getline(headerStream, header) && !header.empty())
			{
				size_t colonPos = header.find(':');
				if (colonPos != std::string::npos)
				{
					std::string key = header.substr(0, colonPos);
					std::string value = header.substr(colonPos + 1);
					trim(key);
					trim(value);
					this->_responseHeaders[key] = value;
				}
			}
		}
		else // If there are no headers, we just return the body of the CGI
			this->_responseBody = this->_cgiBody;

		std::map<std::string, std::string>::iterator contentLengthIt = this->_responseHeaders.find("Content-Length");
		if (contentLengthIt != this->_responseHeaders.end()) // Check if the CGI script returned a Content-Length header
		{
			size_t contentLength = static_cast<size_t>(std::atoi(contentLengthIt->second.c_str()));
			if (contentLength < this->_responseBody.size()) // If the content length is less than the body, we truncate it
				this->_responseBody = this->_responseBody.substr(0, contentLength);
		}
	}

	// Build the location header (For POST 201 created and GET redirection 301)
	// For POST 201 the location header is where the new resource is located just for info/testing purposes
	if (this->_locationHeader != "")
		this->_responseHeaders["Location"] = this->_locationHeader;

	if (isError)
	{
		std::map<int, std::string> errorPages = this->_config.getErrorPages();
		std::string status = statusLine.substr(9, 3);
		std::map<int, std::string>::iterator it = errorPages.find(std::atoi(status.c_str()));
		if (it != errorPages.end())
		{
			this->_responseBody = readFile(it->second);
			this->_responseHeaders["Content-Type"] = this->_determineContentType(it->second);
		}
		else
		{
			this->_responseHeaders["Content-Type"] = "text/html"; // Error pages are always html
			this->_responseBody = this->_generateHTMLPage(isError, statusLine, this->_responseBody);
		}
	}
	else if (!isError && !this->_isCGIRequest())
	{
		bool isDirectoryRequest = this->_requestedPath[this->_requestedPath.size() - 1] == '/';
		bool isAutoindex = this->_config.isAutoindex(this->_config.getLocations(), this->_requestedPath);
		if (statusCode == SUCCESS_200 && this->_method == "GET" && this->_hasIndexFileInResponse()) // If it is a success, we determine the content type because we are serving a file
			this->_responseHeaders["Content-Type"] = this->_determineContentType(this->_responsePath);
		else if (isDirectoryRequest && isAutoindex && !this->_hasIndexFileInResponse()) // If it is a directory request, autoindex is enabled and there is no index file we generate the directory listing
		{
			this->_responseHeaders["Content-Type"] = "text/html";
			this->_responseBody = this->_generateDirectoryListing();
		}
		else
		{
			this->_responseHeaders["Content-Type"] = "text/html"; // Other sucess messages we return an HTML page
			this->_responseBody = this->_generateHTMLPage(isError, statusLine, this->_responseBody);
		}
	}

	this->_responseHeaders["Content-Length"] = toString(this->_responseBody.size()); // Always add the content length after the body is generated(truncated, changed...)

	this->_response = statusLine + "\r\n";
	for (std::map<std::string, std::string>::iterator it = this->_responseHeaders.begin(); it != this->_responseHeaders.end(); ++it)
		this->_response += it->first + ": " + it->second + "\r\n";
	this->_response += "\r\n";
	this->_response += this->_responseBody;

	return this->_response;
}

StatusCode Response::handleMethods()
{
	if (this->_isCGIRequest())
		return this->_handleCGI();
	else if (this->_method == "GET")
		return this->_handleGET();
	else if (this->_method == "POST")
		return this->_handlePOST();
	else if (this->_method == "DELETE")
		return this->_handleDELETE();
	return NO_STATUS_CODE; // Impossible to reach this point
}

bool Response::_isCGIRequest() const
{
	if (this->_requestedPath.empty()) // Prevents segfaults(basic_string)
		return false;

	const std::string mainPath = this->_requestedPath.substr(0, this->_config.getLocationCGIPath(this->_requestedPath).size());
	const std::string locationPath = this->_config.getLocationCGIPath(this->_requestedPath);

	// Check if the requested path is the same as the location path of the location CGI block
	if (mainPath.compare(locationPath) == 0)
		return true;
	return false;
}

StatusCode Response::_handleCGI()
{
	int pipefd[2];
	if (pipe(pipefd) == -1)
		return ERROR_500;

	pid_t pid = fork();
	if (pid == -1)
		return ERROR_500;
	else if (pid == 0)
	{
		close(pipefd[0]); // Close reading in the child

		// Redirect stdout to the pipe
		if (dup2(pipefd[1], STDOUT_FILENO) == -1)
		{
			std::cerr << "Error: duplicating file descriptor for stdout: " << strerror(errno) << std::endl;
			exit(1);
		}
		close(pipefd[1]);

		// If it is a POST request, we need to redirect stdin to the pipe because the CGI script will read from it
		if (this->_method == "POST")
		{
			int inputPipe[2];
			if (pipe(inputPipe) == -1)
			{
				std::cerr << "Error: creating the input pipe: " << strerror(errno) << std::endl;
				exit(1);
			}

			if (dup2(inputPipe[0], STDIN_FILENO) == -1)
			{
				std::cerr << "Error: duplicating file descriptor for stdin: " << strerror(errno) << std::endl;
				exit(1);
			}
			close(inputPipe[0]); // Close the reading end of the pipe

			// Write the body to the pipe
			ssize_t bytesWritten = write(inputPipe[1], this->_requestBody.c_str(), this->_requestBody.size());
			if (bytesWritten == -1)
			{
				std::cerr << "Error: writing to the pipe: " << strerror(errno) << std::endl;
				exit(1);
			}
			close(inputPipe[1]); // Close the writing end of the pipe

			setenv("CONTENT_LENGTH", toString(this->_requestBody.size()).c_str(), 1); // Not necessary in GET requests
		}

		const std::string mainPath = this->_requestedPath.substr(0, this->_config.getLocationCGIPath(this->_requestedPath).size());
		std::string scriptName = this->_requestedPath.substr(this->_config.getLocationCGIPath(this->_requestedPath).size()); // The script name is the part of the URL after the location path
		std::string pathInfo = extractPathInfo(scriptName);
		std::string queryString = extractQueryString(scriptName);

		// Check if script has the extension specified in the location block
		std::string extension = scriptName.substr(scriptName.find_last_of('.'));
		std::string cgiExtension = this->_config.getCGIExtension(mainPath);
		if (extension != cgiExtension)
			exit(3);

		std::string scriptPath = this->_config.getCGIBin(this->_requestedPath) + "/" + scriptName;

		if (access(scriptPath.c_str(), F_OK) != 0) // Check if the CGI script exists
			exit(2);
		else if (access(scriptPath.c_str(), X_OK) != 0) // Check if the CGI script is executable
			exit(3);

		// CGIs need to have some environment variables set
		setenv("REQUEST_METHOD", this->_method.c_str(), 1);
		setenv("SCRIPT_NAME", this->_requestedPath.c_str(), 1);
		setenv("CONTENT_TYPE", _determineContentType(scriptPath).c_str(), 1);
		setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
		setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
		setenv("SCRIPT_NAME", scriptName.c_str(), 1);
		setenv("PATH_INFO", pathInfo.c_str(), 1);
		setenv("QUERY_STRING", queryString.c_str(), 1);

		char *args[] = {const_cast<char *>(scriptPath.c_str()), NULL};
		execve(scriptPath.c_str(), args, environ);

		std::cerr << "Error: executing CGI script: " << strerror(errno) << std::endl;
		exit(1); // If execve fails
	}
	else
	{
		close(pipefd[1]); // Close writing in the parent

		// Read the output from the pipe
		char buffer[1024];
		ssize_t bytesRead;
		while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
			this->_cgiBody.append(buffer, bytesRead);

		close(pipefd[0]); // Close reading in the parent

		int status;
		if (waitpid(pid, &status, 0) == -1)
			return ERROR_500;

		if (WIFEXITED(status))
		{
			int exitStatus = WEXITSTATUS(status);
			if (exitStatus == 2) // CGI script does not exist
				return ERROR_404;
			else if (exitStatus == 3) // CGI script is not executable
				return ERROR_403;
			else if (exitStatus == 1) // Error executing the CGI script
				return ERROR_500;
		}

		if (WIFSIGNALED(status))
			return ERROR_500;

		// If the CGI script didn't return anything, return a 500 error
		if (this->_cgiBody.empty())
			return ERROR_500;
		else
			return SUCCESS_200;
	}

	return NO_STATUS_CODE; // Impossible to reach this point
}

StatusCode Response::_handleGET()
{
	std::string redirect = this->_config.getRedirect(this->_requestedPath);
	if (redirect != "")
	{
		this->_locationHeader = redirect;
		return REDIRECTION_301;
	}
	else if (this->_requestedPath[this->_requestedPath.size() - 1] == '/') // If is a directory just serve the index file in the server config
	{
		std::vector<std::string> indexes = this->_config.getIndexes();
		if (indexes.empty())
			this->_responsePath = this->_config.getRoot() + this->_requestedPath;

		for (std::vector<std::string>::iterator it = indexes.begin(); it != indexes.end(); ++it)
		{
			if (access((this->_config.getRoot() + this->_requestedPath + *it).c_str(), F_OK) == 0)
			{
				this->_responsePath = this->_config.getRoot() + this->_requestedPath + *it;
				break;
			}
		}
		if (this->_responsePath.empty()) // If no index file was found
		{
			if (this->_config.isAutoindex(this->_config.getLocations(), this->_requestedPath)) // If autoindex is enabled, we generate the directory listing
				return SUCCESS_200;
			if (access((this->_config.getRoot() + this->_requestedPath).c_str(), F_OK) != 0) // Check if the directory exists
				return ERROR_404;
			else
				return ERROR_403; // If the directory exists but there is no index file and autoindex is disabled, is forbidden
		}
	}
	else
		this->_responsePath = this->_config.getRoot() + this->_requestedPath;

	// Check if the file has an extension, if not add .html
	if (this->_responsePath.substr(1).find_last_of(".") == std::string::npos) // substr(1) to skip the first "./"root_path
	{
		if (this->_responsePath[this->_responsePath.size() - 1] != '/') // If is a directory, we don't add the extension
			this->_responsePath += ".html";
	}

	if (access(this->_responsePath.c_str(), F_OK) != 0) // Check if the file exists

		return ERROR_404;
	else if (access(this->_responsePath.c_str(), R_OK) != 0) // Check if the file is readable
		return ERROR_403;
	else
		return SUCCESS_200;

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
						if (this->_saveFile(content, filename, savedPath))
						{
							this->_locationHeader = savedPath; // We return where we saved the file but just for info/testing purposes
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
	if (this->_requestedPath.empty())
		return ERROR_400;
	else if (this->_requestHeaders["Content-Length"].find("Content-Length") != std::string::npos &&
			 this->_requestHeaders["Content-Length"] != "0")
		return ERROR_400; // DELETE request should not have a body, if there is present Content-Length header, it should be 0

	std::string fullPath = this->_config.getRoot() + this->_requestedPath;

	// Check if it's a directory
	DIR *dir = opendir(fullPath.c_str());
	bool isDirectory = (dir != NULL);
	if (dir)
		closedir(dir);

	// If it's a directory but doesn't end with '/', append it
	if (isDirectory && this->_requestedPath[this->_requestedPath.size() - 1] != '/')
		fullPath += '/';

	if (access(fullPath.c_str(), F_OK) != 0) // Check if the file exists
		return ERROR_404;
	if (fullPath[fullPath.size() - 1] == '/') // If is a directory, we don't allow to delete it
		return ERROR_403;
	if (access(fullPath.c_str(), W_OK) != 0) // Check if the file is writable
		return ERROR_403;
	if (remove(fullPath.c_str()) == 0)
		return SUCCESS_204;

	return ERROR_500;
}

const std::string Response::_determineContentType(const std::string &filename) const
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

	status = statusLine.substr(9); // Remove "HTTP/1.1 "

	responseBody += "<!DOCTYPE html>\n";
	responseBody += "<html lang=\"en\">\n";
	responseBody += "<head>\n";
	responseBody += "<meta charset=\"UTF-8\">\n";
	responseBody += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
	responseBody += "<title>" + status + "</title>\n";
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

const std::string Response::_generateDirectoryListing()
{
	DIR *dir;
	struct dirent *ent;
	std::string responseBody;
	std::string path = this->_config.getRoot() + this->_requestedPath;

	responseBody += "<!DOCTYPE html>\n";
	responseBody += "<html lang=\"en\">\n";
	responseBody += "<head>\n";
	responseBody += "<meta charset=\"UTF-8\">\n";
	responseBody += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
	responseBody += "<title>Directory Listing - " + this->_requestedPath + "</title>\n";
	responseBody += "<style>\n";
	responseBody += "body { font-family: Arial, sans-serif; line-height: 1.6; color: #333; max-width: 800px; margin: 0 auto; padding: 20px; }\n";
	responseBody += "h1 { color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 10px; }\n";
	responseBody += "ul { list-style-type: none; padding: 0; }\n";
	responseBody += "li { margin: 10px 0; background: #f8f9fa; border-radius: 5px; }\n";
	responseBody += "a { display: block; padding: 10px 15px; color: #2980b9; text-decoration: none; transition: background 0.3s; }\n";
	responseBody += "a:hover { background: #e9ecef; }\n";
	responseBody += ".directory { font-weight: bold; }\n";
	responseBody += ".file { }\n";
	responseBody += "</style>\n";
	responseBody += "</head>\n";
	responseBody += "<body>\n";
	responseBody += "<h1>Directory Listing: " + this->_requestedPath + "</h1>\n";
	responseBody += "<ul>\n";

	if ((dir = opendir(path.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			std::string fileName = std::string(ent->d_name);
			std::string fullPath = path + "/" + fileName;
			struct stat fileStat;

			if (stat(fullPath.c_str(), &fileStat) == 0)
			{
				std::string fileClass = S_ISDIR(fileStat.st_mode) ? "directory" : "file";
				std::string href = fileName;
				if (S_ISDIR(fileStat.st_mode) && fileName != "." && fileName != "..") // If it is a directory, we add a slash at the end
					href += "/";
				responseBody += "<li class=\"" + fileClass + "\"><a href=\"" + href + "\">" + fileName + "</a></li>\n";
			}
		}
		closedir(dir);
	}
	else
		responseBody += "<li>Error reading directory</li>\n";

	responseBody += "</ul>\n";
	responseBody += "</body>\n";
	responseBody += "</html>\n";

	return responseBody;
}

bool Response::_hasIndexFileInResponse() const
{
	// It will be a index file if exists in the response path because we checked it in handleGET
	if (access((this->_responsePath).c_str(), F_OK) == 0)
		return true;
	return false;
}
