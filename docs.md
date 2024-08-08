[Diagrama de ejecucion despues de parseo](https://m4nnb3ll.medium.com/webserv-building-a-non-blocking-web-server-in-c-98-a-42-project-04c7365e4ec7)

[Diagramas entender sockets/ipc](https://medium.com/@md.abir1203/visual-nuts-and-bolts-of-42-webserver-b381abec4df3)

[Evolution http 1 to 3](https://medium.com/@md.abir1203/evolution-of-problem-solving-in-web-protocol-443ee3bb35b4)

[Webserv: A C++ Webserver](https://hackmd.io/@laian/SJZHcOsmT?utm_source=preview-mode&utm_medium=rec#Background-Knowledge)

[Un servidor HTTP en 25 líneas](https://www.youtube.com/watch?v=7GBlCinu9yg)

[42 Seoul tutorial](https://42seoul.gitbook.io/webserv/or-cgi-execute)
## Relación entre IPC y Sockets

- Sockets como IPC: Los sockets son una de las muchas formas en las que los procesos pueden comunicarse entre sí, por lo que se consideran una herramienta de IPC. Son especialmente útiles cuando los procesos no están en la misma máquina o cuando se necesita comunicación a través de una red.

- Diferencias: Mientras que IPC es un concepto amplio que incluye diversas técnicas de comunicación entre procesos, los sockets son solo una de esas técnicas, específica para la comunicación basada en redes o entre procesos locales.

En resumen, sockets son una forma de IPC, pero no todos los mecanismos de IPC son sockets. Los sockets son particularmente útiles cuando necesitas que procesos se comuniquen a través de una red o en la misma máquina usando una interfaz de red.