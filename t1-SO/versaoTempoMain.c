#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int teste_parametro(int argc, char *argv[]);
int potencia(int base, int expoente);
double difTime(struct timespec t0, struct timespec t1);
pid_t forka(void);
void criaCadeiaProcessos(int cont, int qtdProcesso);
void criaArvoreProcessos(int altura_atual, int altura_maxima);
// void criaCadeiaProcessosLaco(int qtdProcesso);
// void criaArvoreProcessosLaco(int altura);

int main(int argc, char *argv[]) {
    struct timespec tempoInicio, tempoFim;
    pid_t processoRaiz;
    int altura, qtdProcesso;

    altura = teste_parametro(argc, argv);
    processoRaiz = getpid();

    /* Teste com cadeia de processos */
    printf("\tCadeia (inicio em PID = %d)\n\n", getpid());
    qtdProcesso = potencia(2, altura + 1) - 1;      // 2^(h+1) - 1
    clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);    
    criaCadeiaProcessos(0, qtdProcesso - 1);
    if (getpid() != processoRaiz)       			// se for filho termina
        return 0;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);
    printf("\n\nTempo execucao processos em cadeia: %lfs\n\n", difTime(tempoInicio, tempoFim));

    printf("-------------------------------------------------\n");

    /* Teste com arvore de processos */
    printf("\tArvore (inicio em PID = %d)\n\n", getpid());
    clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio); 
    criaArvoreProcessos(0, altura);
    if (getpid() == processoRaiz) {
    	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);
    	printf("\n\nTempo execucao processos em arvore: %lfs\n\n", difTime(tempoInicio, tempoFim));
    }

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

void criaCadeiaProcessos(int cont, int qtdProcesso) {
    printf("Possuo PID=%d e PPID=%d\n", getpid(), getppid());
    if (cont < qtdProcesso) {
		if ( forka() )     		 	 //  pai
            wait(0);
        else                    	 // filho
            return criaCadeiaProcessos(cont + 1, qtdProcesso);
    }
    printf("Termino do processo com PID=%d e PPID=%d\n", getpid(), getppid());
}

/*  Ao ser usado
        if ( fork() == 0 || fork() == 0 )
    a. caso o primeiro fork retorne > 0 (pai): o segundo fork eh realizado, 
        criando assim o "filho da direita";
    b. caso o primeiro fork retorne 0 (filho): x OR y eh satisfeito (segundo fork
        nao precisa ser realizado), entao a recursao eh feita e o if, se o processo nao
        for "folha" (teste feito por altura_atual < altura_maxima), eh feito dnv... */
void criaArvoreProcessos(int altura_atual, int altura_maxima) {
    printf("Possuo PID=%d e PPID=%d\n", getpid(), getppid());
    pid_t pid_filho_1 = 0, pid_filho_2 = 0;
    int status;
    if (altura_atual < altura_maxima) {
        if ( (pid_filho_1 = fork()) == 0 || (pid_filho_2 = fork()) == 0 ) {  // filho
            criaArvoreProcessos(altura_atual + 1, altura_maxima);
            return;     // p/ nao imprimir 2x (qdo pai e qdo filho)
        }
    }
    waitpid(pid_filho_1, &status, 0);
    waitpid(pid_filho_2, &status, 0);
    printf("Termino do processo com PID=%d e PPID=%d\n", getpid(), getppid());
}

// criacao dos processos em cadeia utilizando laco inves de recursao
/* void criaCadeiaProcessosLaco(int qtdProcesso) {
    int cont = 0;
    while (cont < qtdProcesso) {
        if ( forka() )           	 // pai
            break;
        cont++;
    }
    printf("Possuo PID=%d e PPID=%d\n", getpid(), getppid());
    wait(0);
    printf("\nTermino do processo com PID=%d e PPID=%d", getpid(), getppid());
} */

// criacao dos processos em arvore utilizando laco inves de recursao
/* void criaArvoreProcessosLaco(int altura) {
    pid_t pid_filho_1 = 0, pid_filho_2 = 0;
    int status;
    for (int i = 0; i < altura; i++) {
        if ( (pid_filho_1 = forka()) == 0 || (pid_filho_2 = forka()) == 0 ) {
            continue;
        }
        break;      
    }
    printf("Sou o processo %d, meu pai eh %d\n", getpid(), getppid());
    waitpid(pid_filho_1, &status, 0);
    waitpid(pid_filho_2, &status, 0);
    printf("Termino do processo %d, com pai %d\n", getpid(), getppid());
    return;
} */