#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
	pid_t idProcesso;

	idProcesso = fork();

	if (idProcesso < 0) {
		fprintf(stderr, "fork falhou\n");
		exit(-1);
	}
	else if (idProcesso == 0)  // filho
		printf("Sou o filho.(PID) = %d, (PPID) = %d\n", getpid(), getppid());
	else	// pai
		printf("Sou o pai. (PID) = %d, (PPID) = %d\n", getpid(), getppid());

	getchar();

	return 0;
}