/* gcc -o numbers_classifier numbers_classifier.c -pthread */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Parametro recebido por cada thread */
typedef struct Parametro {
	int id;
	int qtd_threads;
	int worksize;
} parametro;

/* Resultados obtidos na busca */
typedef struct Retorno {
	int id;
	int defectivos; // qtd de numeros com aliquot_sum < numero
	int abundantes; // qtd de numeros com aliquot_sum > numero
	int perfeitos;  // qtd de numeros com aliquot_sum = numero
} retorno;

void testa_parametros(int argc, char const *argv[], int *qtd_threads, int *worksize);
double difTime(struct timespec t0, struct timespec t1);
double analise_sequencial(int worksize);
double analise_threads(int qtd_threads, int worksize, int tipo);
void *analise_worker_chunk(void *arg);
void *analise_worker_esparsa(void *arg);
int aliquot_sum(int num);
void print_threads_results(struct Retorno **ret, int qtd_threads);

int main(int argc, char const *argv[]) {
	int worksize, qtd_threads;
	testa_parametros(argc, argv, &qtd_threads, &worksize);

	printf("\t---------------------------------\n");
    double tempo_seq = analise_sequencial(worksize);
	printf("\t---------------------------------\n");
	double tempo_threads_chunk = analise_threads(qtd_threads, worksize, 1); // tipo 1 = chunk
	printf("\t---------------------------------\n");
	double tempo_threads_esparsa = analise_threads(qtd_threads, worksize, 2); // tipo 2 = esparsa
	printf("\t---------------------------------\n");
	printf("Tempo da busca sequencial:\t%lfs\n", tempo_seq);
	printf("Tempo da busca com threads:\t%lfs (distribuicao por chunk)\n", tempo_threads_chunk);
	printf("Tempo da busca com threads:\t%lfs (distribuicao esparsa)\n", tempo_threads_esparsa);
	printf("\t---------------------------------\n");

	return 0;
}

void testa_parametros(int argc, char const *argv[], int *qtd_threads, int *worksize) {
	if (argc != 3) {
		fprintf(stderr, "Uso: %s <qtd_threads> <worksize>\n", argv[0]);
		exit(-1);
	}
	if ( (*qtd_threads = atoi(argv[1])) <= 0 ) {
		fprintf(stderr, "%s: qtd_threads deve ser > 0.\n", argv[0]);
		exit(-1);
	}
	if ( (*worksize = atoi(argv[2])) <= 0 ) {
		fprintf(stderr, "%s: worksize deve ser > 0.\n", argv[0]);
		exit(-1);
	}
}

double difTime(struct timespec t0, struct timespec t1) {
	return ((double)t1.tv_sec - t0.tv_sec) + ((double)(t1.tv_nsec-t0.tv_nsec) * 1e-9);
}

double analise_sequencial(int worksize) {
	struct timespec tempoInicio, tempoFim;
	parametro *arg = (parametro*) malloc(sizeof(parametro));

	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);   
	arg->id = 1;
	arg->qtd_threads = 1;
	arg->worksize = worksize;
	retorno *ret = (retorno*) analise_worker_chunk((void*) arg);
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);

    printf("\t\t[D]\t[A]\t[P]\t[WTot]\n");
    printf("* Sequencial:\t%d\t%d\t%d\t%d\n", ret->defectivos, ret->abundantes, ret->perfeitos, worksize);
    free(arg);
    free(ret);
	return difTime(tempoInicio, tempoFim);
}

double analise_threads(int qtd_threads, int worksize, int tipo) {
	struct timespec tempoInicio, tempoFim;
	pthread_t *threads = (pthread_t*) malloc(sizeof(pthread_t) * qtd_threads);
	parametro *arg = (parametro*) malloc(sizeof(parametro) * qtd_threads);
	retorno **ret = (retorno**) malloc(sizeof(retorno*) * qtd_threads);

	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);   
	for (int i = 0; i < qtd_threads; i++) {
		arg[i].id = i + 1; // inicio em thread 1
		arg[i].qtd_threads = qtd_threads;
		arg[i].worksize = worksize;
		if (tipo == 1) { // chunk
			if ( pthread_create(&threads[i], NULL, analise_worker_chunk, (void*) &arg[i]) ) {
				fprintf(stderr, "Erro ao criar thread %d", i);
				exit(-1);		
			}
		} else { // simetrico
			if ( pthread_create(&threads[i], NULL, analise_worker_esparsa, (void*) &arg[i]) ) {
				fprintf(stderr, "Erro ao criar thread %d", i);
				exit(-1);		
			}
		}
	}

	for (int i = 0; i < qtd_threads; i++) {
		if ( pthread_join(threads[i], (void*) &ret[i]) != 0 ) {   
			fprintf(stderr, "Erro no join da thread %d", i);
			exit(-1);	
		}
	}
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);

	print_threads_results(ret, qtd_threads);
	free(threads);
	free(arg);
	for (int i = 0; i < qtd_threads; i++) 
		free(ret[i]);
	free(ret);

	return difTime(tempoInicio, tempoFim);
}

void *analise_worker_chunk(void *arg) {
	parametro *param = (parametro*) arg;;
	int qtd_threads = param->qtd_threads;
	int fatia = (int) (param->worksize / qtd_threads);
	int inicio = fatia * (param->id - 1);
	int fim = (param->id < qtd_threads) ? (inicio +  fatia) : param->worksize;	
	int defectivos, abundantes, perfeitos, sum_divisores;
	defectivos = abundantes = perfeitos = 0;
	/* TODO: Distribuicao + "justa" do resto:
	        lastPos = 0
	        for (...) 
	           inicio = lastPos
	           fim = inicio + fatia
	           if (ws % nthreads) {
	                fim++;
	                ws--;
	           }
	           lastPos = fim
	*/
	for (int i = inicio; i < fim; i++) {
		sum_divisores = aliquot_sum(i);
		if (sum_divisores < i) defectivos++;
		else if (sum_divisores > i) abundantes++;
		else perfeitos++;
	}
	
	retorno *ret= (retorno*) malloc(sizeof(retorno));
	ret->id = param->id;
	ret->defectivos = defectivos;
	ret->abundantes = abundantes;
	ret->perfeitos = perfeitos;
	return ((void*) ret);
}

void *analise_worker_esparsa(void *arg) {
	parametro *param = (parametro*) arg;;
	int defectivos, abundantes, perfeitos, sum_divisores;
	defectivos = abundantes = perfeitos = 0;

	for (int i = param->id - 1; i < param->worksize; i += param->qtd_threads) {
		sum_divisores = aliquot_sum(i);
		if (sum_divisores < i) defectivos++;
		else if (sum_divisores > i) abundantes++;
		else perfeitos++;
	}
	
	retorno *ret= (retorno*) malloc(sizeof(retorno));
	ret->id = param->id;
	ret->defectivos = defectivos;
	ret->abundantes = abundantes;
	ret->perfeitos = perfeitos;
	return ((void*) ret);
}

/* retorna a soma de todos divisores positivos de num (exceto ele proprio) */
int aliquot_sum(int num) { 
	if (num == 0) return -1;
	int sum_divisores = 0; // 
	for (int i = 1; i < (num / 2) + 1; i++) {
		if (num % i == 0) 
			sum_divisores += i;
	}
	return sum_divisores;
}

void print_threads_results(struct Retorno **ret, int qtd_threads) {
	int id, defectivos, abundantes, perfeitos, worksize_thread;
	int total_defectivos, total_abundantes, total_perfetios, total_worksize;
	total_defectivos = total_abundantes = total_perfetios = total_worksize = 0;
    printf("\t\t[D]\t[A]\t[P]\t[WTh]\n");
	for (int i = 0; i < qtd_threads; i++) {
		id = ret[i]->id;
		worksize_thread  = defectivos = ret[i]->defectivos;
		worksize_thread += abundantes = ret[i]->abundantes;
		worksize_thread += perfeitos  = ret[i]->perfeitos;
	    printf("* Thread %d:\t%d\t%d\t%d\t%d\n", id, defectivos, abundantes, perfeitos, worksize_thread);
	    total_defectivos += defectivos;
	    total_abundantes += abundantes;
	    total_perfetios  += perfeitos;
	    total_worksize   += worksize_thread;
	}
    printf("  [TOTAL]\t%d\t%d\t%d\t%d\n", total_defectivos, total_abundantes, total_perfetios, total_worksize);
}
