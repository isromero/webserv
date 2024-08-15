/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:54:49 by isromero          #+#    #+#             */
/*   Updated: 2024/08/15 11:19:55 by isromero         ###   ########.fr       */
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

const std::string Response::handleResponse(StatusCode statusCode)
{
	std::string statusLine, body;

	switch (statusCode)
	{
	case ERROR_400:
		statusLine = "HTTP/1.1 400 Bad Request";
		body = "400 Bad Request: The server cannot process the request due to a client error.";
		break;
	case ERROR_405:
		statusLine = "HTTP/1.1 405 Method Not Allowed";
		body = "405 Method Not Allowed: The method specified in the request is not allowed.";
		break;
	case ERROR_411:
		statusLine = "HTTP/1.1 411 Length Required";
		body = "411 Length Required: The request did not specify the length of its content.";
		break;
	case ERROR_413:
		statusLine = "HTTP/1.1 413 Payload Too Large";
		body = "413 Payload Too Large: The request is larger than the server is willing or able to process.";
		break;
	case ERROR_414:
		statusLine = "HTTP/1.1 414 URI Too Long";
		body = "414 URI Too Long: The URI provided was too long for the server to process.";
		break;
	case ERROR_505:
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
	response += "Content-Length: " + toString(body.size()) + "\r\n\r\n";
	response += body;

	return response;
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

void Response::_handleGET()
{
	// TODO: Refactor and check the requested file here?????
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
			file = "pages/403.html"; // ! TODO: this need to be an error page file variable or something like that
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

StatusCode Response::_handlePOST()
{
	// ! TODO: refactor this method to use the variables(body, headers, etc) of the Response class
	std::string response;
	std::string status = "400 Bad Request";
	std::string responseBody = "400 Bad Request: Malformed POST request.";
	std::string contentType;
	std::string body;
	std::string locationHeader;

	// Obtener el cuerpo y Content-Type
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

	// Verificar Content-Type y manejar el cuerpo
	if (body.empty())
	{
		if (contentType.empty())
			return ERROR_400;
		else
			return ERROR_400;
	}
	else if (contentType == "application/json")
	{
		// Verificación básica de JSON
		if (body.size() >= 2 && body[0] == '{' && body[body.size() - 1] == '}')
		{
			// Crear un nuevo recurso y devolver su ubicación
			status = "201 Created";
			responseBody = "201 Created: Data received and processed successfully.";
			locationHeader = "/new/resource/identifier"; // Ejemplo de URI del nuevo recurso creado // ! TODO: locationHeader will need to be pushed inside of the map headers
		}
		else
		{
			status = "400 Bad Request";
			responseBody = "400 Bad Request: Malformed JSON body.";
		}
	}
	else if (contentType == "application/x-www-form-urlencoded")
	{
		bool valid = true;
		size_t pos = 0;

		// Itera sobre los parámetros separados por '&'
		while (pos < body.length())
		{
			// Encuentra la posición del signo '=' para verificar si existe un valor para la clave
			size_t eqPos = body.find('=', pos);

			// Si no se encuentra un '=', o está al principio o final, es un cuerpo malformado
			if (eqPos == std::string::npos || eqPos == pos || eqPos == body.length() - 1)
			{
				valid = false;
				break;
			}

			// Encuentra el próximo '&' o final de la cadena
			size_t ampPos = body.find('&', eqPos);

			// Si no hay más '&', termina el ciclo
			if (ampPos == std::string::npos)
			{
				break;
			}

			pos = ampPos + 1; // Mueve la posición al siguiente parámetro
		}

		if (valid)
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
		// Verificación básica del boundary
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
	{
		status = "415 Unsupported Media Type";
		responseBody = "415 Unsupported Media Type: The server cannot handle this content type.";
	}

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
