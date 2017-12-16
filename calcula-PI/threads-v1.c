/* gcc -o threads-v1 threads-v1.c -pthread -lm */
/* melhorar precisao: uso de long double e, invés da pow(), usar powl(); */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>

void testa_parametros(int argc, char const *argv[], unsigned int *iteracoes, int *qtd_pt);
void imprime_vet(int *vet, int tam_vet);
double difTime(struct timespec t0, struct timespec t1);
double busca_sequencial(unsigned int iteracoes);
double busca_thread(unsigned int iteracoes, int qtd_pt);
void *soma_faixa_valores(void *info);

typedef struct Parametro {
	int id;
	unsigned int inicio;
	unsigned int fim;
} parametro;

typedef struct Retorno {
	int id;
	double soma;
} retorno;

#define DENOMINADOR_PI			4

int main(int argc, char const *argv[]) {
	unsigned int iteracoes;
	int qtd_pt;		
	testa_parametros(argc, argv, &iteracoes, &qtd_pt);

	double tSeq= busca_sequencial(iteracoes);
	printf("\tTempo execucao calculo sequencial:\t%lfs\n", tSeq);
	printf("\t---------------------------------\n");
	double tThread= busca_thread(iteracoes, qtd_pt);
	printf("\tTempo execucao calculo com threads:\t%lfs\n", tThread);

	return 0;
}

void testa_parametros(int argc, char const *argv[], unsigned int *iteracoes, int *qtd_pt) {
	if (argc != 3) {
		fprintf(stderr, "Uso: %s <iteracoes(precisao)> <qtd_threads>\n", argv[0]);
		exit(-1);
	}
	if ( (*iteracoes = atof(argv[1])) <= 0 ) {
		fprintf(stderr, "%s: iteracoes devem ser um numero positivo no formato notacao cientifica (ex: 1e9)\n", argv[0]);
		exit(-1);
	}
	if ( (*qtd_pt = atoi(argv[2])) <= 0 ) {
		fprintf(stderr, "%s: qtd_threads deve ser > 0\n", argv[0]);
		exit(-1);
	}
}

double difTime(struct timespec t0, struct timespec t1) {
	return ((double)t1.tv_sec - t0.tv_sec) + ((double)(t1.tv_nsec-t0.tv_nsec) * 1e-9);
}

double busca_sequencial(unsigned int iteracoes) {
	struct timespec tempoInicio, tempoFim;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio); 

	parametro info;
	info.id = info.inicio = 0;
	info.fim = iteracoes;
	retorno *ret = (void*) soma_faixa_valores((void*) &info);

	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);
	printf("\tSequencial -> PI = %.8lf\n", (ret->soma * DENOMINADOR_PI));

	return difTime(tempoInicio, tempoFim);
}

double busca_thread(unsigned int iteracoes, int qtd_pt){
	struct timespec tempoInicio, tempoFim;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);   

	int fatia = (int) (iteracoes / qtd_pt);
	pthread_t threads[qtd_pt];
	parametro info[qtd_pt];
	for (int i = 0; i < qtd_pt; i++) {
		info[i].id = i;
		info[i].inicio = fatia * i;
		info[i].fim = ((i + 1) < qtd_pt) ? (info[i].inicio +  fatia) : iteracoes; 

		if ( pthread_create(&threads[i], NULL, soma_faixa_valores, (void*) &info[i]) ) {
			fprintf(stderr, "Erro ao criar thread %d", i);
			exit(-1);		
		}
	}

	retorno *ret;
	double resultado_final = 0;
	for (int i = 0; i < qtd_pt; i++) {
		if ( pthread_join(threads[i], (void*) &ret) == 0 ) {   
			resultado_final += ret->soma;
			free(ret); 
		}
		else {	
			fprintf(stderr, "Erro no join da thread %d", i);
			exit(-1);	
		}
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);

	printf("\tSomatório threads -> PI = %.8lf\n", (resultado_final * DENOMINADOR_PI));

	return difTime(tempoInicio, tempoFim);
}

void *soma_faixa_valores(void *info) {
	parametro *param = (parametro*) info;

	double soma = 0;
	for (long int i = param->inicio; i < param->fim; i++)
		soma += (pow((-1), i)) / ((2*i)+1);
	
	retorno *ret= (retorno*) malloc(sizeof(retorno));
	ret->id = param->id;
	ret->soma = soma;

	return ((void*) ret);
}
