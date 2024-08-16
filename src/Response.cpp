/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adgutier <adgutier@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:54:49 by isromero          #+#    #+#             */
/*   Updated: 2024/08/16 17:23:04 by adgutier         ###   ########.fr       */
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

bool Response::_isCGIRequest(const std::string &requestedFile)
{
	if (requestedFile.find("/cgi-bin/") != std::string::npos ||
		requestedFile.find(".cgi") != std::string::npos ||
		requestedFile.find(".pl") != std::string::npos ||
		requestedFile.find(".py") != std::string::npos)
	{
		return true;
	}
	return false;
}

const std::string &Response::handleMethods()
{
	if (_isCGIRequest(this->_requestedFile)) // Si no hay método HTTP y es un CGI, manejarlo
		this->_handleCGI();
	else if (this->_method == "GET")
		this->_handleGET();
	else if (this->_method == "POST")
		this->_handlePOST();
	else if (this->_method == "DELETE")
		this->_handleDELETE();
	return this->_response;
}

void Response::_handleCGI()
{
	int pipefd[2];
	if (pipe(pipefd) == -1)
	{
		this->_response = "HTTP/1.1 500 Internal Server Error\r\n"
						  "Content-Length: 0\r\n\r\n";
		return;
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		this->_response = "HTTP/1.1 500 Internal Server Error\r\n"
						  "Content-Length: 0\r\n\r\n";
		return;
	}
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
			ssize_t bytesWritten = write(inputPipe[1], this->_body.c_str(), this->_body.size());
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
		setenv("CONTENT_LENGTH", toString(this->_body.size()).c_str(), 1);
		setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
		setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);

		// Ejecutar el CGI
		std::string scriptPath = "./pages" + this->_requestedFile;
		char *args[] = {const_cast<char *>(scriptPath.c_str()), NULL};
		execve(scriptPath.c_str(), args, environ);

		exit(1); // Si execve falla
	}
	else // Proceso padre
	{
		close(pipefd[1]); // Cerramos la escritura en el padre

		// Leer la salida del CGI desde el pipe
		char buffer[1024];
		std::string cgiOutput;
		ssize_t bytesRead;
		while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
		{
			cgiOutput.append(buffer, bytesRead);
		}
		close(pipefd[0]); // Cerramos la lectura una vez finalizada

		waitpid(pid, NULL, 0); // Esperamos al hijo

		// Si no hay salida del CGI, devolver un error
		if (cgiOutput.empty())
		{
			this->_response = "HTTP/1.1 500 Internal Server Error\r\n"
							  "Content-Length: 0\r\n\r\n";
		}
		else
		{
			// Crear la respuesta HTTP con la salida del CGI
			this->_response = "HTTP/1.1 200 OK\r\n"
							  "Content-Type: text/html\r\n"
							  "Content-Length: " +
							  toString(cgiOutput.size()) + "\r\n\r\n" +
							  cgiOutput;
		}
	}
}

void Response::_handlePOST()
{
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
		// Verificación básica de JSON
		if (body.size() >= 2 && body[0] == '{' && body[body.size() - 1] == '}')
		{
			// Crear un nuevo recurso y devolver su ubicación
			status = "201 Created";
			responseBody = "201 Created: Data received and processed successfully.";
			locationHeader = "/new/resource/identifier"; // Ejemplo de URI del nuevo recurso creado
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
