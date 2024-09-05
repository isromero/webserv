#!/bin/bash
echo "Content-type: text/plain"
echo "Content-Length: 9999"
echo "" # ! Necessary to end the header, if is not present, everything will be the body
echo "Hello, World!"
