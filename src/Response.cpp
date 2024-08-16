/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:54:49 by isromero          #+#    #+#             */
/*   Updated: 2024/08/16 22:01:51 by isromero         ###   ########.fr       */
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
		this->_responseBody = "201 Created: The request was successful and a new resource was created.";
		break;
	case ERROR_400:
		statusLine = "HTTP/1.1 400 Bad Request";
		this->_responseBody = "400 Bad Request: The server cannot process the request due to a client error.";
		isError = true;
		break;
	case ERROR_405:
		statusLine = "HTTP/1.1 405 Method Not Allowed";
		this->_responseBody = "405 Method Not Allowed: The method specified in the request is not allowed.";
		isError = true;
		break;
	case ERROR_411:
		statusLine = "HTTP/1.1 411 Length Required";
		this->_responseBody = "411 Length Required: The request did not specify the length of its content.";
		isError = true;
		break;
	case ERROR_413:
		statusLine = "HTTP/1.1 413 Payload Too Large";
		this->_responseBody = "413 Payload Too Large: The request is larger than the server is willing or able to process.";
		isError = true;
		break;
	case ERROR_414:
		statusLine = "HTTP/1.1 414 URI Too Long";
		this->_responseBody = "414 URI Too Long: The URI provided was too long for the server to process.";
		isError = true;
		break;
	case ERROR_415:
		statusLine = "HTTP/1.1 415 Unsupported Media Type";
		this->_responseBody = "415 Unsupported Media Type: The server cannot handle this content type.";
		isError = true;
		break;
	case ERROR_500:
		statusLine = "HTTP/1.1 500 Internal Server Error";
		this->_responseBody = "500 Internal Server Error: The server encountered an unexpected condition that prevented it from fulfilling the request.";
		isError = true;
		break;
	case ERROR_505:
		statusLine = "HTTP/1.1 505 HTTP Version Not Supported";
		this->_responseBody = "505 HTTP Version Not Supported: The HTTP version used in the request is not supported by the server.";
		isError = true;
		break;
	default:
		statusLine = "HTTP/1.1 500 Internal Server Error";
		this->_responseBody = "500 Internal Server Error: The server encountered an unexpected condition that prevented it from fulfilling the request.";
		isError = true;
		break;
	}

	// Build the response

	// if isError is true, the content type will be text/html because we return an error message
	if (isError)
		this->_responseHeaders["Content-Type"] = this->_determineContentType("text/html");
	else
		this->_responseHeaders["Content-Type"] = this->_determineContentType(this->_responseFile);

	this->_responseHeaders["Content-Length"] = toString(this->_responseBody.size());

	// TODO: Create function to create error pages with html???
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
	std::string locationHeader;

	if (this->_requestHeaders["Content-Type"] == "application/json")
	{
		if (body.size() >= 2 && body[0] == '{' && body[body.size() - 1] == '}')
		{
			this->_locationHeader = "pages/uploads/" + getFilenameAndDate("file.json");
			return SUCCESS_201;
		}
		else
			return ERROR_400;
	}
	else if (this->_requestHeaders["Content-Type"] == "application/x-www-form-urlencoded") // This is the typical form data "key=value&key2=value2"
	{
		bool valid = true;
		size_t pos = 0;

		while (pos < body.length())
		{
			size_t eqPos = body.find('=', pos);
			if (eqPos == std::string::npos || eqPos == pos || eqPos == body.length() - 1)
			{
				valid = false;
				break;
			}

			size_t ampPos = body.find('&', eqPos);
			if (ampPos == std::string::npos)
				break;

			pos = ampPos + 1; // Move to the next key=value pair
		}

		if (valid)
			return SUCCESS_201; // TODO: this is not 200??
		else
			return ERROR_400;
	}
	else if (this->_requestHeaders["Content-Type"] == "multipart/form-data") // This is a more complex form data like file uploads
	{
		// Boundary is something like "----WebKitFormBoundary7MA4YWxkTrZu0gW" and it is in the Content-Type header
		size_t boundaryStart = this->_requestHeaders["Content-Type"].find("boundary=");
		if (boundaryStart != std::string::npos)
		{
			std::string boundary = this->_requestHeaders["Content-Type"].substr(boundaryStart + 9);
			if (this->_requestBody.find(boundary) != std::string::npos)
			{
				std::string delimiter = "--" + boundary;
				size_t pos = 0;
				size_t nextPos;

				// TODO: Check functionality, i think is broken, does not make sense using body for extract headers, etc
				while ((pos = this->_requestBody.find(delimiter, pos)) != std::string::npos)
				{
					pos += delimiter.length();
					nextPos = this->_requestBody.find(delimiter, pos);

					if (nextPos == std::string::npos)
						break;

					std::string part = this->_requestBody.substr(pos, nextPos - pos);

					// Process this part
					size_t headerEnd = part.find("\r\n\r\n");
					if (headerEnd == std::string::npos)
						return ERROR_400;

					std::string headers = part.substr(0, headerEnd);
					std::string content = part.substr(headerEnd + 4);

					// Extract filename from headers
					size_t filenamePos = headers.find("filename=\"");
					if (filenamePos != std::string::npos)
					{
						size_t filenameEnd = headers.find("\"", filenamePos + 10);
						if (filenameEnd != std::string::npos)
						{
							std::string filename = headers.substr(filenamePos + 10, filenameEnd - filenamePos - 10);

							// Save the file
							std::string filepath = "pages/uploads/" + getFilenameAndDate(filename);
							std::ofstream file(filepath.c_str(), std::ios::binary); // TODO: hacer una función a parte para guardar archivos y utilizarla ya en el handleResponse?
							if (file.is_open())
							{
								file.write(content.c_str(), content.length());
								file.close();
								this->_locationHeader = filepath;
							}
							else
								return ERROR_500;
						}
					}
					else
						return ERROR_400;
					pos = nextPos;
				}
				return SUCCESS_201;
			}
			else
				return ERROR_400;
		}
		else
			return ERROR_400;
	}
	else if (contentType == "text/plain")
	{
		if (body == "redirect")
		{
			status = "303 See Other";
			locationHeader = "/existing/resource"; // Redirigir a un recurso existente
		}
		else if (body.empty())
		{
			status = "204 No Content";
			responseBody.clear(); // Sin contenido
		}
		else
		{
			status = "200 OK";
			responseBody = "200 OK: Data received and processed successfully.";
		}
	}
	else
		return ERROR_415;

	// Construir la respuesta
	std::ostringstream ss;
	if (status == "204 No Content")
	{
		// Respuesta sin contenido
		response = "HTTP/1.1 " + status + "\r\n" +
				   "Connection: close\r\n" +
				   "\r\n";
	}
	else
	{
		ss << responseBody.size();
		response = "HTTP/1.1 " + status + "\r\n" +
				   "Content-Type: text/plain\r\n" +
				   "Content-Length: " + ss.str() + "\r\n" +
				   (locationHeader.empty() ? "" : "Location: " + locationHeader + "\r\n") +
				   "Connection: close\r\n" + // TODO: Check if we need to close the connection / put in the headers??
				   "\r\n" +
				   responseBody;
	}
	this->_response = response;
}

void Response::_handleDELETE() // TODO: use this to test curl -X DELETE http://localhost:6969/file.html
{
	std::string response;
	std::string status;
	std::string file;

	// Determinar el archivo o recurso a eliminar
	if (this->_requestedFile[this->_requestedFile.size() - 1] == '/') // Si es un directorio, no se elimina
	{
		status = "400 Bad Request";
		response = "400 Bad Request: Directory deletion is not allowed.";
	}
	else
	{
		file = "pages" + this->_requestedFile;

		// Verificar si el archivo existe
		if (access(file.c_str(), F_OK) != 0)
		{
			status = "404 Not Found";
			file = "pages/404.html";
			response = readFile(file);
		}
		else
		{
			// Intentar eliminar el archivo
			if (remove(file.c_str()) != 0)
			{
				status = "403 Forbidden";
				file = "pages/403.html";
				response = readFile(file);
			}
			else
			{
				// En caso de éxito, enviar 204 No Content o 200 OK según la implementación
				// Aquí asumimos que si el archivo se elimina correctamente, no hay contenido adicional
				status = "204 No Content";
				response.clear(); // Sin contenido
			}
		}
	}

	// Construir la respuesta
	if (status == "204 No Content")
	{
		response = "HTTP/1.1 " + status + "\r\n" +
				   "Connection: close\r\n" +
				   "\r\n";
	}
	else
	{
		// Para códigos de estado de error, la respuesta se obtiene del archivo HTML correspondiente
		const std::string contentType = "text/html";
		std::ostringstream ss;
		ss << response.size();
		response = "HTTP/1.1 " + status + "\r\n" +
				   "Content-Type: " + contentType + "\r\n" +
				   "Content-Length: " + ss.str() + "\r\n" +
				   "Connection: close\r\n" +
				   "\r\n" +
				   response;
	}

	this->_response = response;
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
