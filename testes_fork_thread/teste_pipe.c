#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER 256


int main(int argc, char const *argv[]) {
	int fd[2]; // pipe file descriptor :  fd[0] leitura (saida), fd[1] escrita (entrada)
	pid_t pid;

	/*criacao pipe*/
	if (pipe(fd) < 0) {
        fprintf(stderr, "Pipe falhou\n");
		exit(-1);
	}

	if ( (pid = fork()) < 0) {
        fprintf(stderr, "Fork falhou\n");
		exit(-1);
	}
	if (pid > 0)  { 			//pai 
		close(fd[0]); // pai vai escrever, logo fecha leitura do pipe desse lado
		char str[BUFFER] = "Uma string qualquer......";
		printf("\nString enviada pelo Pai (PID=%d e PPID=%d): '%s'\n\n", getpid(), getppid(), str);

		write(fd[1], str, sizeof(str) + 1);	 // escreve string no pipe
		wait(0);	
	}
	else {						// filho
		char str_recebida[BUFFER];
		close(fd[1]); // filho vai ler, logo fecha escrita do pipe

		read(fd[0], str_recebida, sizeof(str_recebida));  // le  oq foi escrito no pipe e armazena em str_recebida

		printf("String lida pelo Filho (PID=%d e PPID=%d): '%s'\n\n", getpid(), getppid(), str_recebida);
		exit(0);
	}

	return 0;
}