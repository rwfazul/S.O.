#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char const *argv[]) {
	pid_t idProcesso = fork();

	if (idProcesso == -1) {
		perror("Fork falhou");
		return 1;		
	}
	if (idProcesso == 0) {
		printf("Sou o filho (%d), meu pai eh %d e vou trocar de imagem\n\n", getpid(), getppid());
		execl("/bin/ls", "ls", NULL, NULL); // nao executa mais a partir daqui
		perror("Filho falou ao executar ls");
		return 1;
	}

	printf("\nSou o pai (%d), estou esperando meu filho (%d) terminar\n", getpid(), idProcesso);
	if (idProcesso != wait (NULL)) {
		perror("Pai falhou a esperar devido a sinal de erro");
		return 1;
	}

	printf("\nSou o processo %d com pai %d e estou saindo\n", getpid(), getppid());


	return(0);
}