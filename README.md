# 🌐 Webserv - HTTP Server from Scratch

Bienvenidos al proyecto **Webserv**, parte del currículo de 42 Network. En este proyecto, desarrollamos nuestro propio servidor HTTP desde cero, siguiendo las especificaciones de los protocolos HTTP/1.1 y algunas funcionalidades adicionales como CGI. Este proyecto es fundamental para comprender el funcionamiento de los servidores web y los protocolos de comunicación en red.

# 🌐 Webserv - HTTP Server from Scratch

Welcome to the Webserv project, part of the 42 Network curriculum. In this project, we develop our own HTTP server from scratch, following the specifications of the HTTP/1.1 protocols and some additional functionalities like CGI. This project is fundamental for understanding the operation of web servers and network communication protocols.

## 📚 What is Webserv?

Webserv is a web server written in C++, capable of handling multiple simultaneous connections using input/output multiplexing. The server follows the HTTP/1.1 protocol and is designed to handle different HTTP methods, manage static and dynamic files through CGI (Common Gateway Interface), and support various configurations through a custom configuration file.

## 🎯 Objectives

- Develop a basic HTTP server that processes requests and responds according to the HTTP/1.1 protocol.
- Implement CGI for executing dynamic scripts.
- Support multiple HTTP methods, such as GET, POST, and DELETE.
- Manage concurrent connections using multiplexing techniques like epoll(), select() or poll().
- Allow dynamic server configuration through a configuration file.

## 🚀 Main Features

- Support for multiple HTTP methods (GET, POST, DELETE).
- CGI implementation to execute scripts (like PHP or Python).
- Input and output multiplexing to handle multiple connections simultaneously.
- Support for different HTTP status codes, including redirects and custom error pages.
- Custom configuration through a configuration file.

## 📜 Basic Theory: HTTP

### 🔄 What is HTTP?

HTTP (Hypertext Transfer Protocol) is a communication protocol used in networks, mainly on the World Wide Web, to transfer data between servers and clients (web browsers, for example). It is a stateless protocol, which means that each request is independent.

### 🌟 Supported HTTP Methods

- GET: Retrieves information from a server (like an HTML page or a file).
- POST: Sends data to the server, typically for processing or storage (forms, files, etc.).
- DELETE: Deletes a specific resource on the server.

### 📑 HTTP Status Codes

The server responds with status codes to indicate the result of the request. Here are some examples:

| Código     | Descripción                                       |
|------------|---------------------------------------------------|
| **200**    | OK: La solicitud se completó con éxito.          |
| **201**    | Created: La solicitud ha sido cumplida y se ha creado un nuevo recurso. |
| **204**    | No Content: La solicitud se completó, pero no hay contenido para enviar. |
| **301**    | Moved Permanently: El recurso solicitado se ha movido de forma permanente a una nueva URL. |
| **400**    | Bad Request: La solicitud no se pudo entender debido a una sintaxis incorrecta. |
| **403**    | Forbidden: El servidor ha entendido la solicitud, pero se niega a autorizarla. |
| **404**    | Not Found: El recurso solicitado no se encuentra. |
| **405**    | Method Not Allowed: El método de la solicitud no está permitido para el recurso solicitado. |
| **408**    | Request Timeout: El servidor agotó el tiempo de espera para la solicitud. |
| **411**    | Length Required: El servidor rechaza la solicitud porque no se especifica la longitud del contenido. |
| **413**    | Payload Too Large: El servidor no puede procesar la solicitud porque el tamaño del contenido es demasiado grande. |
| **414**    | URI Too Long: La URI de la solicitud es demasiado larga para ser procesada. |
| **415**    | Unsupported Media Type: El tipo de medio de la solicitud no está soportado por el servidor. |
| **500**    | Internal Server Error: El servidor encontró un problema al procesar la solicitud. |
| **505**    | HTTP Version Not Supported: El servidor no soporta la versión del protocolo HTTP utilizada en la solicitud. |

## 🔧 Installation and Usage

Follow these steps to clone the project and run it on your local machine. 🚀

### 1. Clone the Repository

First, clone the repository to your machine:

```bash
git clone https://github.com/isromero/webserv.git
cd webserv
```

### 2. Compile the Project

Compile the project using make. This command will generate the webserv executable:

```bash
make
```

Congratulations! 🎉 If everything went well, you now have the server ready to run.

### 3. Run the Server

To run the server, simply use the following command and provide a configuration file:

```bash
./webserv [configuration file]
```

Example:

```bash
./webserv config/webserv.conf
```

⚠️ Note: Make sure you have a valid configuration file in the specified path. We explain how the configuration file works below.

### 4. Configuration File Structure

The configuration file allows you to customize how the server works. Here are some of the parameters you can include:

```nginx
server {
    listen 8080;
    server_name localhost;

    location / {
        root /var/www/html;
        index index.html;
    }

    location /cgi-bin/ {
        cgi_pass /usr/bin/php;
    }

    error_page 404 /errors/404.html;
}
```

- `listen`: The port on which the server will listen for connections (default 8080).
- `server_name`: The server name (localhost, domain, etc.).
- `location`: Defines specific paths and their behavior. Here you can specify the server root, index, or path to CGI scripts.
- `cgi_pass`: Path to the CGI executable (e.g., PHP, Python).
- `error_page`: Custom page for HTTP error codes (e.g., 404 Not Found).

## 🛠️ Usage Example

Once the server is running, open your browser or use curl to make requests:

```bash
curl http://localhost:8080/
```

- GET requests: Retrieve pages or files.
- POST requests: Send data through a form or API.
- DELETE requests: Delete a file or resource on the server.

## 📄 Technical Details

- I/O Multiplexing: We use select() to manage multiple clients simultaneously, without having to resort to threads or processes per client.
- CGI: Support for running dynamic scripts using CGI, allowing integration of languages like PHP or Python into the server.
- Error Handling: When something goes wrong, such as a file not found or an invalid request, the server responds with appropriate error pages.

## 📖 Key Learnings

In this project, we delve into key concepts such as:

- Sockets: Handling network connections.
- I/O Multiplexing: Efficient management of multiple connections.
- HTTP Protocols: Implementation of main HTTP methods and their status codes.
- CGI: Integration of external scripts in the server response.

## 📝 References

- [RFC 1945: HTTP/1.0](https://tools.ietf.org/html/rfc1945)
- [RFC 2616: HTTP/1.1](https://tools.ietf.org/html/rfc2616)
- [RFC 7230: HTTP/1.1 Message Syntax and Routing](https://tools.ietf.org/html/rfc7230)
- [RFC 7231: HTTP/1.1 Semantics and Content](https://tools.ietf.org/html/rfc7231)
- [RFC 7232: HTTP/1.1 Conditional Requests](https://tools.ietf.org/html/rfc7232)
- [RFC 7233: HTTP/1.1 Range Requests](https://tools.ietf.org/html/rfc7233)
- [RFC 7234: HTTP/1.1 Caching](https://tools.ietf.org/html/rfc7234)
- [RFC 7235: HTTP/1.1 Authentication](https://tools.ietf.org/html/rfc7235)
- [RFC 3875: The Common Gateway Interface (CGI) Version 1.1](https://tools.ietf.org/html/rfc3875)

