#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> // Para close()
#include <fstream>  // Para leer archivos
#include <sstream>  // Para std::ostringstream

// Función para leer el contenido de un archivo
std::string readFile(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file) {
        std::cerr << "Error al abrir el archivo: " << filename << std::endl;
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

int main() {
    // Crear el socket
    /*
        socket(AF_INET, SOCK_STREAM, 0): Crea un socket.

        AF_INET: Indica que el socket utiliza el protocolo IPv4.

        SOCK_STREAM: Indica que el socket es de tipo TCP, que es orientado a conexión.

        0: Indica que se usará el protocolo por defecto para el tipo de socket especificado (TCP en este caso).
    */
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error al crear el socket." << std::endl;
        return 1;
    }

    // Configurar la dirección del servidor
    /*
        sockaddr_in serverAddress: Estructura que contiene la dirección del servidor.

        sin_family: Debe ser AF_INET para indicar que es una dirección IPv4.

        sin_addr.s_addr = INADDR_ANY: Indica que el socket debe aceptar conexiones de cualquier interfaz de red.

        sin_port = htons(8080): Especifica el puerto en el que el servidor escuchará, convertido a formato de red con htons (Host to Network Short).

    */
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(6969);

    // Vincular el socket a la dirección del servidor
    /*
        bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)): Vincula el socket a la dirección y puerto especificados en serverAddress.
    */
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error al vincular el socket." << std::endl;
        close(serverSocket);
        return 1;
    }

    // Escuchar por conexiones entrantes
    /*
        listen(serverSocket, 5): Pone el socket en modo escucha para aceptar conexiones entrantes.

        5: Especifica el tamaño de la cola de conexiones pendientes.
    */
    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error al escuchar por conexiones entrantes." << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Servidor HTTP en ejecución. Esperando conexiones..." << std::endl;

    while (true) {
        // Aceptar una conexión entrante
        /*
            accept(serverSocket, NULL, NULL): Acepta una conexión entrante.

            El primer NULL es un puntero a una estructura sockaddr que contendría la dirección del cliente. No nos interesa en este caso.

            El segundo NULL es un puntero a una variable que contendría el tamaño de la estructura sockaddr. Tampoco nos interesa en este caso.
        */
        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == -1) {
            std::cerr << "Error al aceptar la conexión entrante." << std::endl;
            continue;
        }

        // Leer la solicitud del cliente
        /*
            recv(clientSocket, buffer, sizeof(buffer) - 1, 0): Lee datos del socket del cliente.

            clientSocket: El descriptor de archivo del socket del cliente.

            buffer: Un buffer donde almacenar los datos recibidos.

            sizeof(buffer) - 1: El tamaño máximo del buffer menos uno, para dejar espacio para el carácter nulo.

            0: Flags adicionales (ninguna en este caso).
        */
        char buffer[1024];
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead == -1) {
            std::cerr << "Error al leer la solicitud del cliente." << std::endl;
            close(clientSocket);
            continue;
        }

        // Agregar el carácter nulo al final del buffer
        buffer[bytesRead] = '\0';

        // Imprimir la solicitud del cliente
        std::cout << "Solicitud del cliente:" << std::endl;
        std::cout << buffer << std::endl;

        // Crear la respuesta HTTP
        std::string response;
        std::string requestedFile;

        // Parsear la solicitud para obtener el archivo solicitado
        std::string request(buffer);
        size_t pos = request.find("GET /");
        if (pos != std::string::npos) {
            pos += 5; // Move past "GET /"
            size_t endPos = request.find(" ", pos);
            requestedFile = request.substr(pos, endPos - pos);
        }

        // Si la solicitud es para el archivo index.html
        if (requestedFile.empty() || requestedFile == "/") {
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
            if (htmlContent.empty()) {
                response = "HTTP/1.1 500 Internal Server Error\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: 0\r\n\r\n";
            } else {
                std::ostringstream contentLength;
                contentLength << htmlContent.size();

                response = "HTTP/1.1 200 OK\r\n";
                response += "Content-Type: text/html\r\n";
                response += "Content-Length: " + contentLength.str() + "\r\n";
                response += "\r\n";
                response += htmlContent;
            }
        } else {
            // Si la solicitud no es para index.html ni la ruta raíz, servir el archivo de error 500
            std::string errorContent = readFile("404.html");
            if (errorContent.empty()) {
                response = "HTTP/1.1 404 Not Found\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: 0\r\n\r\n";
            } else {
                std::ostringstream contentLength;
                contentLength << errorContent.size();

                response = "HTTP/1.1 500 Internal Server Error\r\n";
                response += "Content-Type: text/html\r\n";
                response += "Content-Length: " + contentLength.str() + "\r\n";
                response += "\r\n";
                response += errorContent;
            }
        }

        // Enviar la respuesta al cliente
        ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
        if (bytesSent == -1) {
            std::cerr << "Error al enviar la respuesta al cliente." << std::endl;
        }

        // Cerrar la conexión con el cliente
        close(clientSocket);
    }

    // Cerrar el socket del servidor
    close(serverSocket);

    return 0;
}