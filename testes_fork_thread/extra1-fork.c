#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pid_t idPaiAtual;
pid_t idAtual;

int main(int argc, char const *argv[]) {
	
	idPaiAtual = getppid();
	idAtual = getpid();

	pid_t idProcesso = fork();

	if (idProcesso < 0) {
		fprintf(stderr, "fork falhou\n");
		exit(-1);
	}
	else if (idProcesso == 0) {
		printf("Sou o filho. PID = %d, PPID = %d\n", getpid(), getppid());
		printf("Sou o filho. Variaveis: IdAtual = %d, idPaiAtual = %d\n", idAtual, idPaiAtual);
	}
	else {
		printf("Sou o pai. PID = %d, PPID = %d\n", getpid(), getppid());
		printf("Sou o pai. Variaveis: IdAtual = %d, idPaiAtual = %d\n", idAtual, idPaiAtual);
	}


	printf("\tPRINTFS EXECUTADOS PELOS DOIS:\n\t");
	printf("PID = %d, PPID = %d\n\t", getpid(), getppid());
	printf("Variaveis: idAtual = %d, IdPaiAtual = %d\n", idAtual, idPaiAtual);

	printf("\n\tTESTE EXECUTADO PELOS DOIS: ");
	if (getpid() == idAtual && getppid() == idPaiAtual) 
		printf("SOU O PAI! PID = %d\n\n", idAtual);
	else {
		printf("SOU O FILHO! PID = %d", getpid());
		printf("\n\tTESTE EXECUTADO PELO FILHO: ");
		if (getppid() == idAtual)
			printf("Deu certo! PPID = %d\n\n", getppid());
		else
			printf("Deu errado! PPID = %d, IdPaiAtual = %d\n\n", getppid(), idPaiAtual);
	}

	getchar();

	return 0;
}