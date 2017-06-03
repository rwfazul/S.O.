#!/bin/bash 

if [ -z "$1" ]; then
	echo "uso: nome arquivo para compilação"
else
	gcc -o $1 $1.c -pthread
	# ./$1	
fi
