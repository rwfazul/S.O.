#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int teste() {
	pid_t pid = fork();
	if (!pid) 
		printf("FUNCAO: (FILHO) PID = %d, PPID = %d\n", getpid(), getppid());
	else 
		printf("FUNCAO: (PAI) PID = %d\n", getpid());
	
	return pid;
}

int main(int argc, char const *argv[]) {
	printf("(PAI) PID = %d\n", getpid());

	int sinal = teste();

	if (!sinal)
		printf("(FILHO) PID = %d, PAI = %d\n", getpid(), getppid());
	else
		printf("(PAI) PID = %d\n", getpid());

	printf("\n");

	return 0;
}