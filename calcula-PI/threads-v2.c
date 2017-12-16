/* gcc -o threads-v2 threads-v2.c -pthread -lm */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>

// Parametro que cada thread recebe 
typedef struct Parametro {
	int id;
	unsigned int inicio;
	unsigned int fim;
} parametro;

// Resultado parcial retornado por cada thread 
typedef struct Retorno {
	int id;
	double soma;
} retorno;

/* teste de parametros e medicoes de tempo */
void testa_parametros(int argc, char const *argv[], int *qtd_threads);
double difTime(struct timespec t0, struct timespec t1);
/* calculo sequencial */
void calculo_sequencial();
/* criacao estrutura da arvore */
/* TODO: Usar apenas uma função de base para criacao das threads usando o vetor de funções para chamar as funcoes de calculo de cada uma das series: typedef (void *) (*serie)(void *info); */
typedef double (*calculo)(int qtd_threads);
void cria_processos(int qtd_threads);
double viete(int qtd_threads);
double nilakantha(int qtd_threads);
double wallis(int qtd_threads);
double leibniz(int qtd_threads);
void carga_thread(struct Parametro *info, int id_thread, int qtd_threads, int max_iteracoes);
/* calculo das series */
void *viete_serie(void *info);
void *nilakantha_serie(void *info);
void *wallis_serie(void *info);
void *leibniz_serie(void *info);
/* facilitar controle da chamada das funções nos processos forkados */
calculo calculos[] = {
	viete,
	nilakantha,
	wallis,
	leibniz
};

#define QUANTIDADE_PROCESSOS		4		  // numero de series = numero de processos
/* Testar com outros valores para aumentar a precisao do resultado */
#define VIETE_ITERACOES				15        // default 15
#define NILAKANTHA_ITERACOES		500		  // default 500
#define WALLIS_ITERACOES			900000000 // default 900000000
#define LEIBNIZ_ITERACOES			900000000 // default 900000000

int main(int argc, char const *argv[]) {
	int qtd_threads;		
	testa_parametros(argc, argv, &qtd_threads);

	printf("\n\t\tCALCULO POR THREADS\n");
	printf("\t----------------------------------------\n");
	cria_processos(qtd_threads);


	printf("\n\t\tCALCULO SEQUENCIAL\n");
	printf("\t----------------------------------------\n");
	calculo_sequencial();

	return 0;
}

void testa_parametros(int argc, char const *argv[], int *qtd_threads) {
	if (argc != 2) {
		fprintf(stderr, "Uso: %s <qtd_threads>\n", argv[0]);
		exit(-1);
	}
	if ( (*qtd_threads = atoi(argv[1])) <= 0 ) {
		fprintf(stderr, "%s: qtd_threads deve ser > 0\n", argv[0]);
		exit(-1);
	}
}

double difTime(struct timespec t0, struct timespec t1) {
	return ((double)t1.tv_sec - t0.tv_sec) + ((double)(t1.tv_nsec-t0.tv_nsec) * 1e-9);
}

void calculo_sequencial() {
	struct timespec tempoInicio, tempoFim;
	parametro info;
	retorno *ret;
	double resultado_final;
	info.id = info.inicio = 0;

	/* VIETE */
	info.fim = VIETE_ITERACOES;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);
	ret = (void*) viete_serie((void*) &info);
	resultado_final = 2 / ret->soma; // correcao final da formula
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);
	printf("\tSérie de Viete -> PI = %1.8lf\n", resultado_final);
	printf("\tTempo de execucao do calculo: %lfs\n", difTime(tempoInicio, tempoFim));
	printf("\t----------------------------------------\n");
	free(ret);

	/* NILAKANTHA */
	info.fim = NILAKANTHA_ITERACOES;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);
	ret = (void*) nilakantha_serie((void*) &info);
	resultado_final = 3 + (ret->soma + (3/4)) * 4; // corecao final da formula
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);
	printf("\tSérie de Nilakantha -> PI = %1.8lf\n", resultado_final);
	printf("\tTempo de dexecucao do calculo: %lfs\n", difTime(tempoInicio, tempoFim));
	printf("\t----------------------------------------\n");
	free(ret);

	/* WALLIS */
	info.fim = WALLIS_ITERACOES;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);
	ret = (void*) wallis_serie((void*) &info);
	resultado_final = 4 * ret->soma; // corecao final da formula
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);
	printf("\tSérie de Wallis -> PI = %1.8lf\n", resultado_final);
	printf("\tTempo de execucao do calculo: %lfs\n", difTime(tempoInicio, tempoFim));
	printf("\t----------------------------------------\n");
	free(ret);

	/* LEIBNIZ */
	info.fim = LEIBNIZ_ITERACOES;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);
	ret = (void*) leibniz_serie((void*) &info);
	resultado_final = 4 * ret->soma; // corecao final da formula
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);
	printf("\tSérie de Leibniz -> PI = %1.8lf\n", resultado_final);
	printf("\tTempo de execucao do calculo: %lfs\n", difTime(tempoInicio, tempoFim));
	printf("\t----------------------------------------\n");
	free(ret);
}

void cria_processos(int qtd_threads) {
	int status, pid;

	for (int i = 0; i < QUANTIDADE_PROCESSOS; i++) {
		if ( (pid = fork()) ) { // pai
    		waitpid(pid, &status, 0); // executa uma serie por vez
		} else { // filho
			double tThread = calculos[i](qtd_threads); // chama funcao de cada serie que ira criar as threads
			printf("\tTempo de execucao do calculo: %lfs\n", tThread);
			printf("\t----------------------------------------\n");
			exit(0);
		}
	}
}

double viete(int qtd_threads){
	struct timespec tempoInicio, tempoFim;
	pthread_t threads[qtd_threads];
	parametro info[qtd_threads];
	retorno *ret;
	double resultado_final = 1;

	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);   
	for (int i = 0; i < qtd_threads; i++) {
		carga_thread(&info[i], i, qtd_threads, VIETE_ITERACOES);
		if ( pthread_create(&threads[i], NULL, viete_serie, (void*) &info[i]) ) {
			fprintf(stderr, "Erro ao criar thread %d", i);
			exit(-1);		
		}
	}

	for (int i = 0; i < qtd_threads; i++) {
		if ( pthread_join(threads[i], (void*) &ret) == 0 ) {   
			resultado_final *= ret->soma;
			free(ret); 
		}
		else {	
			fprintf(stderr, "Erro no join da thread %d", i);
			exit(-1);	
		}
	}
   	resultado_final = 2 / resultado_final; // corecao final da formula
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);
	printf("\tSérie de Viete -> PI = %1.8lf\n", resultado_final);

	return difTime(tempoInicio, tempoFim);
}

double nilakantha(int qtd_threads){
	struct timespec tempoInicio, tempoFim;
	pthread_t threads[qtd_threads];
	parametro info[qtd_threads];
	retorno *ret;
	double resultado_final = 0;

	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);   
	for (int i = 0; i < qtd_threads; i++) {
		carga_thread(&info[i], i, qtd_threads, NILAKANTHA_ITERACOES);
		if ( pthread_create(&threads[i], NULL, nilakantha_serie, (void*) &info[i]) ) {
			fprintf(stderr, "Erro ao criar thread %d", i);
			exit(-1);		
		}
	}

	for (int i = 0; i < qtd_threads; i++) {
		if ( pthread_join(threads[i], (void*) &ret) == 0 ) {   
			resultado_final += ret->soma;
			free(ret); 
		}
		else {	
			fprintf(stderr, "Erro no join da thread %d", i);
			exit(-1);	
		}
	}
	resultado_final = 3 + (resultado_final + (3/4)) * 4; // corecao final da formula
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);
	printf("\tSérie de Nilakantha -> PI = %1.8lf\n", resultado_final);

	return difTime(tempoInicio, tempoFim);
}

double wallis(int qtd_threads){
	struct timespec tempoInicio, tempoFim;
	pthread_t threads[qtd_threads];
	parametro info[qtd_threads];
	retorno *ret;
	double resultado_final = 1;
	
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);   
	for (int i = 0; i < qtd_threads; i++) {
		carga_thread(&info[i], i, qtd_threads, WALLIS_ITERACOES);
		if ( pthread_create(&threads[i], NULL, wallis_serie, (void*) &info[i]) ) {
			fprintf(stderr, "Erro ao criar thread %d", i);
			exit(-1);		
		}
	}

	for (int i = 0; i < qtd_threads; i++) {
		if ( pthread_join(threads[i], (void*) &ret) == 0 ) {   
			resultado_final *= ret->soma;
			free(ret); 
		}
		else {	
			fprintf(stderr, "Erro no join da thread %d", i);
			exit(-1);	
		}
	}
	resultado_final *= 4; // corecao final da formula
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);
	printf("\tSérie de Wallis -> PI = %1.8lf\n", resultado_final);

	return difTime(tempoInicio, tempoFim);
}

double leibniz(int qtd_threads){
	struct timespec tempoInicio, tempoFim;
	pthread_t threads[qtd_threads];
	parametro info[qtd_threads];
	retorno *ret;
	double resultado_final = 0;
	
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);   
	for (int i = 0; i < qtd_threads; i++) {
		carga_thread(&info[i], i, qtd_threads, LEIBNIZ_ITERACOES);
		if ( pthread_create(&threads[i], NULL, leibniz_serie, (void*) &info[i]) ) {
			fprintf(stderr, "Erro ao criar thread %d", i);
			exit(-1);		
		}
	}

	for (int i = 0; i < qtd_threads; i++) {
		if ( pthread_join(threads[i], (void*) &ret) == 0 ) {   
			resultado_final += ret->soma;
			free(ret); 
		}
		else {	
			fprintf(stderr, "Erro no join da thread %d", i);
			exit(-1);	
		}
	}
	resultado_final *= 4; // corecao final da formula
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);
	printf("\tSérie de Leibniz -> PI = %1.8lf\n", resultado_final);

	return difTime(tempoInicio, tempoFim);
}

void carga_thread(struct Parametro *info, int id_thread, int qtd_threads, int max_iteracoes) {
	int fatia = (int) (max_iteracoes / qtd_threads);
	info->id = id_thread;
	info->inicio = fatia * id_thread;
	info->fim = ((id_thread + 1) < qtd_threads) ? (info->inicio +  fatia) : max_iteracoes;	
}

/* APlICACAO DAS FORMULAS */
void *viete_serie(void *info) {
	parametro *param = (parametro*) info;
	double inicio, soma, f; 
	inicio = param->inicio;
	soma = 1;
	for (double i = param->fim; i > inicio; i--) {
		f = 2;
		for (double j = 1; j < i; j++)
			f = 2 + sqrt(f);	
		f = sqrt(f);
		soma = soma * f / 2;
	}

	retorno *ret= (retorno*) malloc(sizeof(retorno));
	ret->id = param->id;
	ret->soma = soma;

	return ((void*) ret);	
}

void *nilakantha_serie(void *info) { 
	parametro *param = (parametro*) info;
	double soma = 0;
	// implementado como somatorio pra facilitar controle dos sinais
	for (double i = param->inicio; i < param->fim; i++) 
		soma += (pow((-1), i)) / (pow(((2*i)+3), 3) - ((2*i)+3));

	retorno *ret= (retorno*) malloc(sizeof(retorno));
	ret->id = param->id;
	ret->soma = soma;

	return ((void*) ret);
}

void *wallis_serie(void *info) {
	parametro *param = (parametro*) info;
	double soma = 1;

   	for (double i = param->inicio + 3; i <= (param->fim + 2); i += 2)   {
   		//printf(" (%1.lf)/(%1.f) * (%1.lf)/(%1.f)\n", i-1, i, i+1, i);
   		soma = soma * ((i - 1) / i) * (( i + 1) / i);
   	} 

	retorno *ret= (retorno*) malloc(sizeof(retorno));
	ret->id = param->id;
	ret->soma = soma;

	return ((void*) ret);	
}

void *leibniz_serie(void *info) {
	parametro *param = (parametro*) info;
	double soma = 0;
	// implementado como somatorio pra facilitar controle dos sinais
	for (double i = param->inicio; i < param->fim; i++)
		soma += (pow((-1), i)) / ((2*i)+1);
	
	retorno *ret= (retorno*) malloc(sizeof(retorno));
	ret->id = param->id;
	ret->soma = soma;

	return ((void*) ret);
}
