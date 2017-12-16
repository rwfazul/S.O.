#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
	printf("\nOla mundo eu sou o processo forkado que deu execev\n");
	// exit(31); 
	// _exit(valor); // caso falhe
	return 30; // poderia ter usado pipe (viria como parametro do main.c ou uma share memory)
}
