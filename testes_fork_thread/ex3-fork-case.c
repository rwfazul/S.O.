#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
	pid_t idProcesso;


	switch ( idProcesso = fork() ) {
		case -1: exit(-1);
		case 0: printf("Sou o filho. PID = %d, PPID = %d\n", getpid(), getppid());
			break;
		default: printf("Sou o pai. PID = %d, PPID = %d\n", getpid(), getppid());
			break;
	}

	printf("Printf executado pelos dois. PID = %d\n\n", getpid());
	getchar();

	return 0;
}