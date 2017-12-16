#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char const *argv[]) {
	int retorno;
	pid_t id = fork();

	if ( id == 0) { // filho
		printf("Sou o filho e vou trocar de imagem:\n");
		execl("./fork", NULL, NULL, NULL);
		perror("Filho falhou no execl");
		exit(-1);
	} 
	printf("Sou o pai vou esperar...\n");
	printf("%d %d\n", getpid(), id);
	if ( id != wait(&retorno) ) {
		perror("Pai falhou ao esperar");	
		exit(-1);
	}
	printf("Sucesso!!!\n");
	if (WIFEXITED(retorno))
        printf("Exit status do filho = %d\n", WEXITSTATUS(retorno));
    
	return 0;
}