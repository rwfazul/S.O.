#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

int *valida_parametros(int argc, char const *argv[], int *altura);
void cria_filhos(int num_filhos, int filho_atual);

int main(int argc, char const *argv[]) {	
	int raiz = getpid(), altura = 0;
	int *num_filhos = valida_parametros(argc, argv, &altura);

    printf("Inicio em: PID=%d\n", getpid());
    int criou = 0; 
    for (int i = 0; i < altura && !criou; i++) { // itera os niveis da arvore
		int pai = getpid();
		cria_filhos(num_filhos[i], 0);
		if (getpid() == pai)
			criou = 1; // garante que o processo que ja teve seus filhos criados nao os crie novamente
    }
    printf("Fim PID=%d...\n", getpid());

    if (getpid() == raiz)
    	free(num_filhos); // se for a raiz desaloca

    return 0;
}

/* Valida se usuario digitou todos os parametros corretamente 
	Retorno: Vetor com o numero de filhos por nivel de cada processo */
int *valida_parametros(int argc, char const *argv[], int *altura) {
	if (argc < 2) {
		fprintf(stderr, "Uso: %s <num filhos por nivel>\n", argv[0]);
		exit(-1);
	}
	*altura = (argc - 1);
	int *num_filhos = (int*) malloc(sizeof(int) * (*altura));
	for (int i = 0; i < (*altura); i++) {
		int qtd = atoi(argv[i+1]);
		if (qtd <= 0) {
			fprintf(stderr, "Parametro %d eh invalido...\n", i);
			exit(-1);
		}
		num_filhos[i] = qtd;
	}
	return num_filhos;
}

/* Processo que chamar essa funcao tera filhos criados = num_filhos */
void cria_filhos(int num_filhos, int filho_atual) {
	int pid;
	if (filho_atual < num_filhos)  {
		if ( pid = fork() ) {
			cria_filhos(num_filhos, filho_atual + 1);
		}
		else {
			printf("PID=%d  PPID=%d\n", getpid(), getppid());
			return;
		}
	}
	while (wait(NULL) > 0); // espera todos os filhos terminarem
}