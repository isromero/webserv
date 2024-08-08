#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

// Función para leer el contenido de un archivo
std::string readFile(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	if (!file)
	{
		std::cerr << "Error al abrir el archivo: " << filename << std::endl;
		return "";
	}
	return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// TODO: Change errors for strrerror and gai_strerror?

int main()
{
	// AF_INET: ipv4
	// SOCK_STREAM: TCP
	// 0: Default (0 = TCP)
	int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketfd == -1)
	{
		std::cerr << "Error: creating the socket" << std::endl;
		return (1);
	}

	// Create the server address
	sockaddr_in serverAddress;

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(6969); // htons converts the port number to network byte order

	// Bind the socket to the address and port
	if (bind(socketfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
	{
		std::cerr << "Error: binding the socket to the address" << std::endl;
		close(socketfd);
		return (1);
	}

	// Listen for incoming connections, 1000 is the maximum queue length
	if (listen(socketfd, 1000) < 0)
	{
		std::cerr << "Error: listening for incoming connections" << std::endl;
		close(socketfd);
		return (1);
	}
	std::cout << "Waiting for incoming connections..." << std::endl;

	while (1)
	{
		// Accept the incoming connection
		int clientSocket = accept(socketfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
		if (clientSocket == -1)
		{
			std::cerr << "Error: accepting the incoming connection" << std::endl;
			close(clientSocket);
			continue;
		}

		// Read the request from the client the request is stored in the buffer
		std::string requestData;
		char buffer[1024];
		ssize_t bytesRead;

		while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
		{
			request.append(buffer, bytesRead);

			// Verify if the request has ended
			if (request.find("\r\n\r\n") != std::string::npos)
				break;
		}

		std::cout << "Request:" << std::endl;
		std::cout << request << std::endl;

		// Prepare the response
		std::string response;
		std::string requestedFile;
		size_t pos = request.find("GET /");
		if (pos != std::string::npos)
		{
			pos += 5; // Move past "GET /"
			size_t endPos = request.find(" ", pos);
			requestedFile = request.substr(pos, endPos - pos);
		}

		// Si la solicitud es para el archivo index.html
		if (requestedFile.empty() || requestedFile == "/")
		{
			response = "HTTP/1.1 200 OK\r\n";
			response += "Content-Type: text/html\r\n";
			response += "Content-Length: 13\r\n"; // Longitud de "Hello, World!"
			response += "\r\n";
			response += "Hello, World!";
		}
		else if (requestedFile == "index.html")
		{
			// Si la solicitud es para index.html, servir el contenido de index.html
			std::string htmlContent = readFile("index.html");
			if (htmlContent.empty())
			{
				response = "HTTP/1.1 500 Internal Server Error\r\n"
						   "Content-Type: text/plain\r\n"
						   "Content-Length: 0\r\n\r\n";
			}
			else
			{
				std::ostringstream contentLength;
				contentLength << htmlContent.size();

				response = "HTTP/1.1 200 OK\r\n";
				response += "Content-Type: text/html\r\n";
				response += "Content-Length: " + contentLength.str() + "\r\n";
				response += "\r\n";
				response += htmlContent;
			}
		}
		else
		{
			// Si la solicitud no es para index.html ni la ruta raíz, servir el archivo de error 500
			std::string errorContent = readFile("404.html");
			if (errorContent.empty())
			{
				response = "HTTP/1.1 404 Not Found\r\n"
						   "Content-Type: text/plain\r\n"
						   "Content-Length: 0\r\n\r\n";
			}
			else
			{
				std::ostringstream contentLength;
				contentLength << errorContent.size();

				response = "HTTP/1.1 500 Internal Server Error\r\n";
				response += "Content-Type: text/html\r\n";
				response += "Content-Length: " + contentLength.str() + "\r\n";
				response += "\r\n";
				response += errorContent;
			}
		}

		// Send the response
		ssize_t bytesSent = send(clientSocket, response.c_str(), response.size(), 0);
		if (bytesSent == -1)
			std::cerr << "Error: sending the response" << std::endl;
		else
		{
			std::cout << "Response:" << std::endl;
			std::cout << response << std::endl;
		}
		// Close the client socket
		close(clientSocket);
	}
	// Close the server socket
	close(socketfd);
	return (0);
}
