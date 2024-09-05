#!/bin/bash
echo "Content-type: text/plain"
echo ""

echo "Request Method: $REQUEST_METHOD"
echo "Query String: $QUERY_STRING"
echo "PATH INFO: $PATH_INFO"

if [ "$REQUEST_METHOD" = "POST" ]; then
    echo "POST Data:"
    cat
fi 

echo "Hello, World!"
