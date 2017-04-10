#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
	int pid;

	printf("Sou o processo original com PID = %d e PPID = %d.\n", getpid(), getppid());

	if ( pid = fork() )  {
		printf("Sou o pai. PID = %d, PPID = %d, CHILD = %d\n", getpid(), getppid(), pid);
	} else {
		// printf("Sou o filho. PID = %d, PPID = %d\n", getpid(), getppid());
		sleep(5);
		printf("Sou o filho. PID = %d, PPID = %d\n", getpid(), getppid());
	}

	printf("Processo com PID = %d terminou!\n", getpid());
	
	return 0;
}