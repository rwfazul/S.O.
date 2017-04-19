#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int teste_parametro(int argc, char *argv[]);
double difTime(struct timespec t0, struct timespec t1);
pid_t forka(void);
void criaArvoreProcessos(int altura_atual, int altura_maxima, pid_t pidRaiz);
struct timespec tempoInicio, tempoFim;

int main(int argc, char *argv[]) {
    pid_t processoRaiz;
    int altura, qtdProcesso;

    altura = teste_parametro(argc, argv);
    processoRaiz = getpid();

    /* Teste com arvore de processos */
    // printf("\tArvore (inicio em PID = %d)\n\n", getpid());
    criaArvoreProcessos(0, altura, processoRaiz);

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

/*  Ao ser usado
        if ( fork() == 0 || fork() == 0 )
    a. caso o primeiro fork retorne > 0 (pai): o segundo fork eh realizado, 
        criando assim o "filho da direita";
    b. caso o primeiro fork retorne 0 (filho): x OR y eh satisfeito (segundo fork
        nao precisa ser realizado), entao a recursao eh feita e o if, se o processo nao
        for "folha" (teste feito por altura_atual < altura_maxima), eh feito dnv... */
void criaArvoreProcessos(int altura_atual, int altura_maxima, pid_t pidRaiz) {
    if (getpid() == pidRaiz)
		clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);    

    // printf("Possuo PID=%d e PPID=%d\n", getpid(), getppid());
    pid_t pid_filho_1 = 0, pid_filho_2 = 0;
    int status;
    if (altura_atual < altura_maxima) {
        if ( (pid_filho_1 = fork()) == 0 || (pid_filho_2 = fork()) == 0 ) {  // filho
            criaArvoreProcessos(altura_atual + 1, altura_maxima, pidRaiz);
            return;
        }
    }
    waitpid(pid_filho_1, &status, 0);
    waitpid(pid_filho_2, &status, 0);

    if (getpid() == pidRaiz) {
    	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);
    	// printf("Termino execucao arvore, PID=%d\n", getpid());
    	// printf("\n\nTempo execucao processos em arvore: %lfs\n\n", difTime(tempoInicio, tempoFim));
        printf("%lf\n", difTime(tempoInicio, tempoFim));
    } else {						// filho
    	// printf("Termino do processo com PID=%d e PPID=%d\n", getpid(), getppid());
    	exit(0);
    }
}
