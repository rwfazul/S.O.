#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char const *argv[]) {
	int status;
	pid_t child;

	printf("Sou o pai. PID = %d, PPID = %d\n\n", getpid(), getppid());

	if ( !(child = fork()) ) {
		printf("Sou o filho. PID = %d, PPID = %d\n", getpid(), getppid());
	} else {
		printf("Aguardando meu filho terminar... ");
		waitpid(child, &status, 0);
		printf("Acabou! Status = %d.\n", status);
	}

	printf("Printf executado pelos dois. Meu id eh %d.\n\n", getpid());

	// getchar();

	return 0;
}