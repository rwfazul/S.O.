#include <stdio.h>
#include <unistd.h>


int main(int argc, char const *argv[]) {
	pid_t idPai;
	pid_t idProcesso;

	idPai = getppid();
	idProcesso = getpid();

	printf("PPID = %d\nPID = %d\n\n", idPai, idProcesso);

	getchar();

	return 0;
}