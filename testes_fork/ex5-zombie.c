#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
	int pid;

	printf("Sou o processo pai, PID = %d e PPID = %d, e vou criar um filho.\n", getpid(), getppid());
	pid = fork();

	if (pid == -1) {
		fprintf(stderr, "fork falhou\n");
		exit(-1);
	}
	else if (pid == 0) {
		printf("Sou o filho, PID = %d e PPID = %d, e vou dormir por 10 seg.", getpid(), getppid());
		sleep(10);
		printf("Sou %d e acordei! Vou terminar agora. Ops virei um 'zumbi'!", getpid());
		exit(0);
	}
	else
		for(;;);

    exit(0);

	return 0;
}
