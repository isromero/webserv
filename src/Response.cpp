/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:54:49 by isromero          #+#    #+#             */
/*   Updated: 2024/08/20 18:11:27 by isromero         ###   ########.fr       */
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
		if (this->_method == "GET")
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

	if (!this->_isCGIRequest())
	{
		// Build the headers and body for not cgi requests
		// if isError is true, the content type will be text/html because we return an error message
		if (isError)
		{
			this->_responseHeaders["Content-Type"] = "text/html"; // Error pages are always html
			this->_responseBody = this->_generateHTMLPage(isError, statusLine, this->_responseBody);
		}
		else if (!isError)
		{
			if (statusCode == SUCCESS_200 && this->_method == "GET") // If it is a success, we determine the content type because we are serving a file
				this->_responseHeaders["Content-Type"] = this->_determineContentType(this->_responseFile);
			else
			{
				this->_responseHeaders["Content-Type"] = "text/html"; // Other sucess messages we return an HTML page
				this->_responseBody = this->_generateHTMLPage(isError, statusLine, this->_responseBody);
			}
		}
		if (this->_locationHeader != "")
			this->_responseHeaders["Location"] = this->_locationHeader;
		this->_responseHeaders["Content-Length"] = toString(this->_responseBody.size());
	}
	else if (this->_isCGIRequest()) // TODO gestionar los ERRORES 500 de CGI al igual que el anterior if
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
			else if (contentLength > this->_responseBody.size())
				this->_responseHeaders["Content-Length"] = toString(this->_responseBody.size()); // If the content length is greater than the body, we adjust it
		}
		else // If the CGI script didn't return a Content-Length header, we add it getting the size of the body
			this->_responseHeaders["Content-Length"] = toString(this->_responseBody.size());
	}

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

bool Response::_isCGIRequest()
{
	// TODO: Add more extensions
	if (this->_requestedFile.find("/cgi-bin/") != std::string::npos ||
		this->_requestedFile.find(".cgi") != std::string::npos ||
		this->_requestedFile.find(".pl") != std::string::npos ||
		this->_requestedFile.find(".py") != std::string::npos)
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
	else if (pid == 0) // Proceso hijo (CGI)
	{
		close(pipefd[0]); // Cerramos la lectura en el hijo

		// Redirigimos stdout al pipe
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);

		// Si el método es POST, redirigir stdin
		if (this->_method == "POST")
		{
			int inputPipe[2];
			if (pipe(inputPipe) == -1)
				exit(1); // Error al crear el pipe

			dup2(inputPipe[0], STDIN_FILENO); // Redirigir stdin al extremo de lectura del pipe
			close(inputPipe[0]);			  // Cerramos la lectura ya que está redirigida

			// Escribir los datos POST al extremo de escritura del pipe
			ssize_t bytesWritten = write(inputPipe[1], this->_requestBody.c_str(), this->_requestBody.size());
			if (bytesWritten == -1)
			{
				// Manejar el error de la escritura si es necesario
				perror("Error writing to CGI input pipe");
				// Podrías salir del proceso hijo si hay un error crítico
				exit(1);
			}
			close(inputPipe[1]); // Cerramos la escritura después de enviar los datos
		}

		// Configurar las variables de entorno
		setenv("REQUEST_METHOD", this->_method.c_str(), 1);
		setenv("SCRIPT_NAME", this->_requestedFile.c_str(), 1);
		setenv("QUERY_STRING", this->_request.c_str(), 1);
		setenv("CONTENT_TYPE", _determineContentType(this->_requestedFile).c_str(), 1);
		setenv("CONTENT_LENGTH", toString(this->_requestBody.size()).c_str(), 1);
		setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
		setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
		std::string scriptPath = "./pages" + this->_requestedFile;
		setenv("PATH_INFO", scriptPath.c_str(), 1);

		char *args[] = {const_cast<char *>(scriptPath.c_str()), NULL};
		execve(getenv("PATH_INFO"), args, environ);

		exit(1); // Si execve falla
	}
	else // Proceso padre
	{
		close(pipefd[1]); // Cerramos la escritura en el padre

		// Leer la salida del CGI desde el pipe
		char buffer[1024];
		ssize_t bytesRead;
		while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
			this->_cgiBody.append(buffer, bytesRead);

		close(pipefd[0]); // Cerramos la lectura una vez finalizada

		waitpid(pid, NULL, 0); // Esperamos al hijo

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
	else if (this->_requestHeaders["Content-Length"].find("Content-Length") != std::string::npos && this->_requestHeaders["Content-Length"] != "0")
		return ERROR_400;												   // DELETE request should not have a body, if there is present Content-Length header, it should be 0
	else if (this->_requestedFile[this->_requestedFile.size() - 1] == '/') // If is a directory, we don't allow to delete it
		return ERROR_403;
	else
	{
		file = "pages" + this->_requestedFile;

		if (access(file.c_str(), F_OK) != 0) // Check if the file exists
			return ERROR_404;
		else if (access(file.c_str(), W_OK) != 0) // Check if the file is writable
			return ERROR_403;
		else if (remove(file.c_str()) == 0)
			return SUCCESS_204;
		else
			return ERROR_500;
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
