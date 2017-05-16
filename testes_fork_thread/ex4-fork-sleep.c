#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
	pid_t idProcesso;
	int i;
	printf("Iniciando o programa... Meu ID = %d, meu Pai = %d\n", getpid(), getppid());

	switch ( idProcesso = fork() ) {
		case -1: exit(-1);
		case 0: for (i = 0; i < 4; i++) {
					printf("Sou o filho. PID = %d, PPID = %d\n", getpid(), getppid());
					sleep(4);
				}
			break;
		default: for (i = 0; i < 4; i++) {
					printf("Sou o pai. PID = %d, PPID = %d\n", getpid(), getppid());
					sleep(4);
				}
			break;
	}

	printf("\n\nO processo com ID = %d terminou!\n", getpid());

	return 0;
}