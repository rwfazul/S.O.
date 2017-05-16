#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
	pid_t idProcesso;

	idProcesso = fork();

	if (idProcesso < 0) {
		fprintf(stderr, "erro no fork\n");
		exit(-1);
	}
	else if (idProcesso == 0)
		printf("Sou o filho. (PID) = %d, (PPID) = %d\n", getpid(), getppid());
	else
		printf("Sou o pai. (PID) = %d, (PPID) = %d\n", getpid(), getppid());

	printf("Printf executado em ambos. (PID) = %d, (PPID) = %d\n\n", getpid(), getppid());

	getchar();

	return 0;
}
