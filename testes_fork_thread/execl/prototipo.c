#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define QUANTIDADE_PROCESSOS 	4

int main(int argc, char const *argv[]) {
	int retorno, pid;
	char *series[] = { "viete", "nilakantha", "wallis", "leibniz" };
	
	for (int i = 0; i < QUANTIDADE_PROCESSOS; i++) {
		if ( (pid = fork()) ) { // pai
			if ( pid != wait(&retorno) ) {
				perror("Pai falhou ao esperar");	
				exit(-1);
			}
			if (WIFEXITED(retorno))
        			printf("Exit status do filho %d = %d\n", i+1, WEXITSTATUS(retorno));
		} else { // filho
			// execl(("%s.c", series[i]), ("%s.c", series[i]), (char const*) NULL); // segundo parametro = argv[0]
			execl(("%s.c", series[i]), argv[1], argv[2], (char const*) NULL);
			perror("Filho falhou no execl");
			exit(-1);
		}
	} 

	return 0;
}



