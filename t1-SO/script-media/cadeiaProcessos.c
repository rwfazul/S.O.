#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int teste_parametro(int argc, char *argv[]);
int potencia(int base, int expoente);
double difTime(struct timespec t0, struct timespec t1);
pid_t forka(void);
void criaCadeiaProcessos(int cont, int qtdProcesso, pid_t pidRaiz);
struct timespec tempoInicio, tempoFim;

int main(int argc, char *argv[]) {
    pid_t processoRaiz;
    int altura, qtdProcesso;

    altura = teste_parametro(argc, argv);
    qtdProcesso = potencia(2, altura + 1) - 1;      // 2^(h+1) - 1
    processoRaiz = getpid();

    /* Teste com cadeia de processos */
    // printf("\tCadeia (inicio em PID = %d)\n\n", getpid());
    criaCadeiaProcessos(0, qtdProcesso - 1, processoRaiz);

    return 0;
}

int teste_parametro(int argc, char *argv[]) {
    int altura;
    if (argc != 2) {
        fprintf(stderr, "Uso: %s altura\n", argv[0]);
        exit(-1);
    }
    if ( (altura = atoi(argv[1])) <= 0 ) {
        fprintf(stderr, "%s: altura deve ser > 0\n", argv[0]);
        exit(-1);
    }
    return altura;
}

int potencia(int base, int expoente) {
    return (expoente == 1) ? base : base * potencia(base, expoente - 1);
}

double difTime(struct timespec t0, struct timespec t1){
    return ((double)t1.tv_sec - t0.tv_sec) + ((double)(t1.tv_nsec-t0.tv_nsec) * 1e-9);
}
 
pid_t forka(void) {
    pid_t pid;
    if ( (pid = fork()) < 0 ) {
        fprintf(stderr, "Fork falhou\n");
        exit(-1);
    }
    return pid;
}

void criaCadeiaProcessos(int cont, int qtdProcesso, pid_t pidRaiz) {
    if (getpid() == pidRaiz)
		clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);    

    // printf("Possuo PID=%d e PPID=%d\n", getpid(), getppid());
    if (cont < qtdProcesso) {
		if ( forka() )     		 	 //  pai
            wait(0);
        else                    	 // filho
            return criaCadeiaProcessos(cont + 1, qtdProcesso, pidRaiz);
    }

    if (getpid() == pidRaiz) {
    	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);
    	// printf("Termino execucao cadeia, PID=%d\n", getpid());
    	// printf("\n\nTempo execucao processos em cadeia: %lfs\n\n", difTime(tempoInicio, tempoFim));
    	printf("%lf\n", difTime(tempoInicio, tempoFim));
    } else {						// filho
    	// printf("Termino do processo com PID=%d e PPID=%d\n", getpid(), getppid());
    	exit(0);
    }
}

