// gcc -o arv_proc arv_proc.c 
// ./arv_proc <altura> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int valida_parametros(int argc, char const *argv[]);
double dif_time(struct timespec t0, struct timespec t1);
double branch(int altura, pid_t raiz);
double livre(int altura, pid_t raiz);
void imprime_resultado(double t_branch, double t_livre);

int main(int argc, char const *argv[]) {	
    int altura = valida_parametros(argc, argv);
    pid_t raiz = getpid();
	
    printf("\n----------- Branch -----------\n");
    double t_branch = branch(altura, raiz);
    printf("--- Fim criacao por Branch ---\n\n");
    printf("----------- Livre ------------\n");
    double t_livre = livre(altura, raiz);
    printf("----- Fim criacao Livre ------\n");

    imprime_resultado(t_branch, t_livre);
    printf("Fim PID=%d (root)\n\n", getpid());	
	
    return 0;
}

/*  Retorno = altura da arvore a ser criada. */
int valida_parametros(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <altura>\n", argv[0]);
        exit(-1);
    }

    int altura = atoi(argv[argc-1]);
    if (altura <= 0) {
        fprintf(stderr, "Altura deve ser > 0\n");
        exit(-1);
    }

    return altura;
}

/*  Retorno = diferenca de tempo (em segundos) entre t0 e t1. */
double dif_time(struct timespec t0, struct timespec t1) {
    return ((double) t1.tv_sec - t0.tv_sec) + ((double) (t1.tv_nsec-t0.tv_nsec) * 1e-9);
}

/* Cria arvore um branch por vez (pai cria um filho e espera ele executar...), cria em "pre-ordem", finaliza em "pos-ordem".
   Retorno: tempo para criar estrutura. */
double branch(int altura, pid_t raiz) {
    pid_t pid_filho_1, pid_filho_2;
    int eh_pai = 0, nivel = 0;
    struct timespec tempo_inicio, tempo_fim;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tempo_inicio);
    printf("n=%d\tPID=%d (root)\n", nivel++, getpid());
    for (int i = 0; i < altura && !eh_pai; i++) {
        if ( (pid_filho_1 = fork()) == 0 ) {     // 1st filho
            printf("n=%d\tC[%d, %d]\n", nivel, getpid(), getppid());
        } else {                                 // pai
            waitpid(pid_filho_1, NULL, 0);
            if ( (pid_filho_2 = fork()) == 0 ) { // 2nd filho
                printf("n=%d\tC[%d, %d]\n", nivel, getpid(), getppid());
            } else {                             // pai
                waitpid(pid_filho_2, NULL, 0);   
                eh_pai = 1;
            }	
        }
        nivel++;
    }

    if (getpid() != raiz) {
        printf("\tT[%d, %d]\n", getpid(), getppid());
        exit(0);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &tempo_fim);

    return dif_time(tempo_inicio, tempo_fim);
}

/* Cria arvore "cheia" (pode criar filhos dos dois branchs antes de esperar).
   Retorno: tempo para criar estrutura. */
double livre(int altura, pid_t raiz) {
    pid_t pid_filho_1, pid_filho_2;
    int eh_pai = 0, nivel = 0;
    struct timespec tempo_inicio, tempo_fim;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tempo_inicio);
    printf("n=%d\tPID=%d (root)\n", nivel++, getpid());

    for (int i = 0; i < altura && !eh_pai; i++) {
        if ( (pid_filho_1 = fork()) == 0 || (pid_filho_2 = fork()) == 0 ) {   // filho
            printf("n=%d\tC[%d, %d]\n", nivel, getpid(), getppid());
        } else {
            waitpid(pid_filho_1, NULL, 0);
            waitpid(pid_filho_2, NULL, 0);
            eh_pai = 1;
        }
       nivel++;
    }
    if (getpid() != raiz) {
        printf("\tT[%d, %d]\n", getpid(), getppid());
        exit(0);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &tempo_fim);

    return dif_time(tempo_inicio, tempo_fim);
}

/* Imprime tempos de execução das duas arvores. */
void imprime_resultado(double t_branch, double t_livre) {
    printf ("\nTempo execucao 'Branch': %lfs\n", t_branch);
    printf ("Tempo execucao 'Livre':  %lfs\n", t_livre);

    if (t_branch < t_livre) {
    	printf("'Branch' foi mais rapido por %lfs (aprox. %1.lfx mais rapido)\n", (t_livre - t_branch), (t_livre/t_branch));
    } else if (t_livre < t_branch) {
    	printf("'Livre' foi mais rapido por %lfs (aprox. %1.lfx mais rapido)\n", (t_branch - t_livre), (t_branch/t_livre));
    } else {
    	printf("Tempos foram iguais!");
    }  
}
