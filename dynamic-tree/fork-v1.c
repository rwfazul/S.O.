#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {	
    int pid = 0;

    printf("Inicio: PID=%d\n", getpid());

    for (int i = 1; i < argc; i++) { // varre argv[]
        int qtd = atoi(argv[i]); // qtd processos a ser criada = argv[i]
        for (int i = 0; i < qtd; i++) {
            if ( pid = fork() ) { // pai
                continue; // cria seus outros filhos
            } else {             // filho	
                printf("PID=%d  PPID=%d\n", getpid(), getppid());
                break;  // vai pra iteracao em que o filho vira 'pai'
            }
        }
        if ( pid ) {   // pai
            while (wait(NULL) > 0); // espera todos os filhos terminarem
            break;
        }
    }

    printf("Fim PID=%d...\n", getpid());

    return 0;
}

