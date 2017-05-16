#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

int main(int argc, char const *argv[]) {
	pid_t idProcesso;
	int estado, count = 0;

	idProcesso = fork();
	if (idProcesso < 0)
		exit(errno);
	else if (idProcesso != 0) {
		printf("Sou o pai, PID = %d, e vou esperar filho\n", getpid());
		wait(&estado);
		printf("Sou o pai, PID = %d, e esperei pelo filho %d\n", getpid(), idProcesso);
	}
	else if (idProcesso == 0) {
		while (count < 5) {
			printf("Sou o filho (%d), meu pai eh %d\n", getpid(), getppid());
			sleep(2);
			count++;
		}
	}

	printf("Sou %d e estou terminando.\n\n", getpid());
	exit(0);

	return 0;
}