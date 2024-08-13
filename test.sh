#!/bin/bash

# URL del servidor al que se enviarán las solicitudes
URL="http://localhost:6969"

# 1. POST con Content-Type: application/json (válido)
echo "Enviando POST con application/json (válido)..."
curl -X POST "$URL" \
     -H "Content-Type: application/json" \
     -d '{"nombre":"Juan","edad":30}'

echo -e "\n"

# 2. POST con Content-Type: application/json malformado
echo "Enviando POST con application/json malformado..."
curl -X POST "$URL" \
     -H "Content-Type: application/json" \
     -d '{"nombre": "Juan", "edad": 30' # Falta un cierre de llave

echo -e "\n"

# 3. POST sin Content-Type
echo "Enviando POST sin Content-Type..."
curl -X POST "$URL" \
     -d '{"nombre":"Juan","edad":30}'

echo -e "\n"

# 4. POST con Content-Type: application/x-www-form-urlencoded (válido)
echo "Enviando POST con application/x-www-form-urlencoded (válido)..."
curl -X POST "$URL" \
     -H "Content-Type: application/x-www-form-urlencoded" \
     -d "nombre=Juan&edad=30"

echo -e "\n"

# 5. POST con Content-Type: application/x-www-form-urlencoded malformado
echo "Enviando POST con application/x-www-form-urlencoded malformado..."
curl -X POST "$URL" \
     -H "Content-Type: application/x-www-form-urlencoded" \
     -d "nombre&edad=30" # Falta el signo igual para el primer parámetro

echo -e "\n"


# 6. POST con Content-Type: multipart/form-data malformado
echo "Enviando POST con multipart/form-data malformado..."
curl -X POST "$URL" \
     -H "Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW" \
     --data "------WebKitFormBoundary7MA4YWxkTrZu0gW
Content-Disposition: form-data; name=\"nombre\"
Juan
------WebKitFormBoundary7MA4YWxkTrZu0gW--" # Falta la nueva línea entre el nombre del campo y el valor

echo -e "\n"

# 7. POST con Content-Type: text/plain (válido)
echo "Enviando POST con text/plain (válido)..."
curl -X POST "$URL" \
     -H "Content-Type: text/plain" \
     -d "Este es un texto simple."

echo -e "\n"

# 8. POST a una URL incorrecta
echo "Enviando POST a una URL incorrecta..."
curl -X POST "http://localhost:9999" \
     -H "Content-Type: application/json" \
     -d '{"nombre":"Juan","edad":30}'

echo -e "\n"


# Fin del script
echo "Pruebas completadas."
