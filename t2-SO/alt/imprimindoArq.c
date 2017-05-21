/* gcc -o rhauani-t2SO rhauani-t2SO.c -pthread -lm */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>

void testa_parametros(int argc, char const *argv[], int *tam_vet, int *qtd_pt);
int *get_vetor_random(int tam_vet);
void imprime_vet(int *vet, int tam_vet);
double difTime(struct timespec t0, struct timespec t1);
double busca_sequencial(int *vet, int tam_vet);
double busca_fork(int *vet, int tam_vet, int qtd_pt);
double busca_thread(int *vet, int tam_vet, int qtd_pt);
void *procura_intervalo(void *info);
int eh_primo(int num);

typedef struct Parametro {
	int id_pt;                             // identificação thread ou processo (pid)
	int *vet;
	int indice_inicio; 
	int indice_fim;
} parametro;

typedef struct Retorno {
	int id_pt;
	int qtd_primos;
} retorno;

#define MAX_RANDOM			500	           // valor maximo dos numeros gerados para preencher o vet

int main(int argc, char const *argv[]) {
	int tam_vet, qtd_pt;		
	testa_parametros(argc, argv, &tam_vet, &qtd_pt);

	int *vet = get_vetor_random(tam_vet);
	imprime_vet(vet, tam_vet);
	double tSeq= busca_sequencial(vet, tam_vet);
	double tThread= busca_thread(vet, tam_vet, qtd_pt);
	double tProcesso = busca_fork(vet, tam_vet, qtd_pt);
	printf("\n\tTempo execucao busca sequencial:    %lfs\n", tSeq);
	printf("\tTempo execucao busca com threads:   %lfs\n", tThread);
	printf("\tTempo execucao busca com processos: %lfs\n\n", tProcesso);
	

	FILE *fSeq = fopen("sequencial.txt", "a");
	fprintf(fSeq, "%lf\n", tSeq);
	fclose(fSeq);

	FILE *fThread = fopen("thread.txt", "a");
	fprintf(fThread, "%lf\n", tThread);
	fclose(fThread);

	FILE *fProc = fopen("processo.txt", "a");
	fprintf(fProc, "%lf\n", tProcesso);
	fclose(fProc); 

	free(vet);

	return 0;
}

void testa_parametros(int argc, char const *argv[], int *tam_vet, int *qtd_pt) {
	if (argc != 3) {
		fprintf(stderr, "Uso: %s tam_vet qtd_proc/threads\n", argv[0]);
		exit(-1);
	}
	if ( (*tam_vet = atoi(argv[1])) <= 0 ) {
		fprintf(stderr, "%s: tam_vet deve ser > 0\n", argv[0]);
		exit(-1);
	}
	if ( (*qtd_pt = atoi(argv[2])) <= 0 ) {
		fprintf(stderr, "%s: qtd_proc/threads deve ser > 0\n", argv[0]);
		exit(-1);
	}
	if ( (*qtd_pt > *tam_vet) ) {			
		fprintf(stderr, "%s: tam_vet deve ser maior que qtd_proc\n", argv[0]);
		exit(-1);
	}
}

int *get_vetor_random(int tam_vet) {
	srand( (unsigned)time(NULL) );

	int* vet = (int*) malloc(tam_vet * sizeof(int));
	if (vet == NULL) {
		fprintf(stderr, "Memoria insuficiente para alocacao do vetor\n");
		exit(-1);
	}

	for (int i = 0; i < tam_vet; i++)
		vet[i] = 1 + rand() % MAX_RANDOM;

	return vet;
}

void imprime_vet(int *vet, int tam_vet) {
	printf("\n");
	for (int i = 0; i < tam_vet; i++) 
		printf("%d ", vet[i]);
	printf("\n\n");
}

double difTime(struct timespec t0, struct timespec t1) {
	return ((double)t1.tv_sec - t0.tv_sec) + ((double)(t1.tv_nsec-t0.tv_nsec) * 1e-9);
}

double busca_sequencial(int *vet, int tam_vet) {
	struct timespec tempoInicio, tempoFim;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio); 

	int qtd_primos = 0;

	for (int i = 0; i < tam_vet; i++)
		qtd_primos += eh_primo(vet[i]);

	printf("Sequencial -> %d numeros primos!\n", qtd_primos);

	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);
	return difTime(tempoInicio, tempoFim);
}

double busca_fork(int *vet, int tam_vet, int qtd_pt) {
	struct timespec tempoInicio, tempoFim;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);   

	int fatia = (int) (tam_vet / qtd_pt);
	parametro info[qtd_pt];
	retorno ret_proc[qtd_pt];
	// int fd[qtd_pt][2];            // desempenho igual a unico pipe usado no escopo do for
	for (int i = 0; i < qtd_pt; i++) {
		int fd[2];                  // pipe file descriptor :  fd[0] leitura (saida), fd[1] escrita (entrada)
		pid_t pid;
		if (pipe(fd) < 0) {         // criacao pipe
			fprintf(stderr, "Pipe falhou\n");
			exit(-1);
		}
		if ( (pid = fork()) < 0) {
			fprintf(stderr, "Fork falhou\n");
			exit(-1);
		}

		if (pid > 0) {                                          //pai 
			wait(0);                // evita defunct/zombie
			close(fd[1]);           // pai vai ler, logo fecha escrita do pipe
			read(fd[0], &ret_proc[i], sizeof(retorno));	// le  oq foi escrito no pipe e armazena no vetor de retornos
			// podia imprimir resultado individual aqui, mas desempenho eh similar a imprimir tudo após o for 
		}
		else {                                                  // filho
			info[i].id_pt = getpid();
			info[i].vet = vet;
			info[i].indice_inicio =  fatia * i;
			// se sobra algum resto fica para o ultimo processo (par/impar)
			info[i].indice_fim = ((i + 1) < qtd_pt) ? (info[i].indice_inicio +  fatia) : tam_vet;
			
			close(fd[0]);           // filho vai escrever, logo fecha a leitura do pipe
			write(fd[1], (retorno*) procura_intervalo((void*) &info[i]), sizeof(retorno));	// escreve retorno da funcao procura_intervalo
			exit(0);                // fim do filho (evita voltar no loop)
		}
	}

	/* Processo principal ("processo pai") imprime resultados de todos processos criados a partir dele */
	 for (int i = 0; i < qtd_pt; i++) 
		printf("Processo %d -> %d numeros primos!\n", ret_proc[i].id_pt, ret_proc[i].qtd_primos); 

	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);   
	return difTime(tempoInicio, tempoFim);
}

double busca_thread(int *vet, int tam_vet, int qtd_pt){
	struct timespec tempoInicio, tempoFim;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);   

	int fatia = (int) (tam_vet / qtd_pt);
	pthread_t threads[qtd_pt];
	parametro info[qtd_pt];
	retorno *ret;
	for (int i = 0; i < qtd_pt; i++) {
		info[i].id_pt = i;
		info[i].vet = vet;
		info[i].indice_inicio =  fatia * i;
		info[i].indice_fim = ((i + 1) < qtd_pt) ? (info[i].indice_inicio +  fatia) : tam_vet; 
		if ( pthread_create(&threads[i], NULL, procura_intervalo, (void*) &info[i]) ) {
			fprintf(stderr, "Erro ao criar thread %d", i);
			exit(-1);		
		}
	}

	/* Processo principal ("thread pai") imprime resultados de todas threads criadas a partir dele */
	for (int i = 0; i < qtd_pt; i++) {
		if ( pthread_join(threads[i], (void*) &ret) == 0 ) {      // retorno da funcao 
			printf("Thread %d -> %d numeros primos!\n", (ret->id_pt + 1), ret->qtd_primos);
			free(ret);
		}
		else {
			fprintf(stderr, "Erro no join da thread %d", i);
			exit(-1);	
		}
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);
	return difTime(tempoInicio, tempoFim);
}

void *procura_intervalo(void *info) {
	parametro *param = (parametro*) info;
	int qtd_primos = 0;

	for (int i = param->indice_inicio; i < param->indice_fim; i++) 
		qtd_primos += eh_primo(param->vet[i]);
	
	retorno *ret= (retorno*) malloc(sizeof(retorno));
	ret->id_pt = param->id_pt;
	ret->qtd_primos = qtd_primos;

 	return ((void*) ret);
}

int eh_primo(int num) {
   	for (int cont = 2; cont <= sqrt(num); cont++) 
		if (num % cont == 0)
			return 0;
	return (num == 1) ? 0 : 1;
}
