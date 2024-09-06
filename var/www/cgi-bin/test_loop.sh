#!/bin/bash

# Imprimir encabezados HTTP necesarios
echo "Content-Type: text/plain"
echo "Cache-Control: no-cache"
echo "Expires: 0"
echo ""

# Mensaje inicial
echo "Entering infinite loop..."

# Bucle infinito
while true; do
    # Imprime un mensaje de estado cada 10 segundos
    echo "Looping..."
    sleep 10
done
