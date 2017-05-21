#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>

void testa_parametros(int argc, char const *argv[], int *tam_vet, int *qtd_pt);
int* get_vetor_random(int tam_vet);
void imprime_vet(int *vet, int tam_vet);
double difTime(struct timespec t0, struct timespec t1);
double busca_sequencial(int *vet, int tam_vet);
double busca_fork(int *vet, int tam_vet, int qtd_pt, pid_t pidRaiz);
double busca_thread(int *vet, int tam_vet, int qtd_pt, pid_t pidRaiz);
void *procura_intervalo(void *info);
int eh_primo(int num);

typedef struct Param {
	int id_pt;
	int tam_vet;
	int qtd_pt;
	int *vet;
} param;

#define MAX_RANDOM		10

int main(int argc, char const *argv[]) {
	int tam_vet, qtd_pt;		
	testa_parametros(argc, argv, &tam_vet, &qtd_pt);

	int *vet = get_vetor_random(tam_vet);
	imprime_vet(vet, tam_vet);
	double tSeq= busca_sequencial(vet, tam_vet);
	double tThread= busca_thread(vet, tam_vet, qtd_pt, getpid());
	double tProcesso = busca_fork(vet, tam_vet, qtd_pt, getpid());


	printf("\n\tTempo execucao busca sequencial:    %lfs\n", tSeq);
	printf("\tTempo execucao busca com threads:   %lfs\n", tThread);
	printf("\tTempo execucao busca com processos: %lfs\n\n", tProcesso);

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

int* get_vetor_random(int tam_vet) {
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

/* double busca_fork(int *vet, int tam_vet, int qtd_pt, pid_t pidRaiz) {
	struct timespec tempoInicio, tempoFim;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);   

	int inicio = 0;
	int *ini_leitura = &inicio;
	for (int i = 0; i < qtd_pt; i++) {
		// calula intervalo de busca que cada processo vai fazer no vet. Se for o ultimo processo vai ate o final (N/M onde M = impar)
		int fim_leitura = ((i + 1) < qtd_pt) ? (int) (tam_vet / qtd_pt) + *ini_leitura : tam_vet; 
		int qtd_primos = 0;

		for (; *ini_leitura < fim_leitura; *ini_leitura = *ini_leitura + 1) 
			qtd_primos += eh_primo(vet[*ini_leitura]);
		printf("Processo PID %d -> %d numeros primos!\n", getpid(), qtd_primos);

		int fd[2]; // pipe file descriptor :  fd[0] leitura (saida), fd[1] escrita (entrada)
		pid_t pid;
		if (pipe(fd) < 0) { // criacao pipe
	        fprintf(stderr, "Pipe falhou\n");
			exit(-1);
		}
		if ( (pid = fork()) < 0) {
	        fprintf(stderr, "Fork falhou\n");
			exit(-1);
		}

		if (pid > 0)  { 							//pai 
			close(fd[0]); // pai vai escrever, logo fecha leitura do pipe desse lado
			write(fd[1], ini_leitura, sizeof(int));	 // escreve ini_leitura no pipe
			wait(0);
			break;	
		}
		else {										// filho
			close(fd[1]); // filho vai ler, logo fecha escrita do pipe
			read(fd[0], ini_leitura, sizeof(int));  // le  oq foi escrito no pipe e armazena no novo ini_leitura
		}
	}

	if (getpid() != pidRaiz)
		exit(0);
	
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);   
	return difTime(tempoInicio, tempoFim);
} */


double busca_fork(int *vet, int tam_vet, int qtd_pt, pid_t pidRaiz) {
	struct timespec tempoInicio, tempoFim;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);   

	for (int i = 0; i < qtd_pt; i++) {		
	    int fd[2]; // pipe file descriptor :  fd[0] leitura (saida), fd[1] escrita (entrada)
		pid_t pid;		
		if (pipe(fd) < 0) { // criacao pipe
	        fprintf(stderr, "Pipe falhou\n");
			exit(-1);
		} 
		if ( (pid = fork()) < 0) {
	        fprintf(stderr, "Fork falhou\n");
			exit(-1);
		}
		if (pid > 0)  { 							//pai 
			close(fd[0]); // pai vai escrever, logo fecha leitura do pipe desse lado
			param info;
			info.tam_vet = tam_vet;
			info.qtd_pt = qtd_pt;
			info.vet = vet;
			info.id_pt = i;
			write(fd[1], &info, sizeof(param));	 // escreve ini_leitura no pipe 
            wait(0);
			continue;
		}
		else {										// filho
			param arg;
			close(fd[1]); // filho vai ler, logo fecha escrita do pipe
			read(fd[0], &arg, sizeof(param));  // le  oq foi escrito no pipe e armazena no novo ini_leitura
			int fatia = (int) (arg.tam_vet / arg.qtd_pt);
			int inicio_leitura = (int) fatia * arg.id_pt;
			int fim_leitura = ((arg.id_pt + 1) < arg.qtd_pt) ? (inicio_leitura +  fatia) : arg.tam_vet;
			int qtd_primos = 0;

			for (int i = inicio_leitura; i < fim_leitura; i++) 
				qtd_primos += eh_primo(arg.vet[i]); 
			printf("Processo PID %d -> %d numeros primos!\n", getpid(), qtd_primos);

			exit(0);
		}

	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);   
	return difTime(tempoInicio, tempoFim); 
}

double busca_thread(int *vet, int tam_vet, int qtd_pt, pid_t pidRaiz){
	struct timespec tempoInicio, tempoFim;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoInicio);   

	pthread_t threads[qtd_pt];
	param info[qtd_pt];
	for (int i = 0; i < qtd_pt; i++) {
		info[i].id_pt = i;
		info[i].vet = vet;
		info[i].tam_vet = tam_vet;
		info[i].qtd_pt = qtd_pt;
		if (pthread_create(&threads[i], NULL, procura_intervalo, (void*) &info[i])) {
	        fprintf(stderr, "Erro ao criar thread %d", i);
         	exit(-1);		
		}
	}

	for (int i = 0; i < qtd_pt; i++)
		pthread_join(threads[i], NULL);

	clock_gettime(CLOCK_MONOTONIC_RAW, &tempoFim);
	return difTime(tempoInicio, tempoFim);
}

void *procura_intervalo(void *info) {
	param *arg = (param*) info;
	int fatia = (int) (arg->tam_vet / arg->qtd_pt);
	int inicio_leitura = (int) fatia * arg->id_pt;
	int fim_leitura = ((arg->id_pt + 1) < arg->qtd_pt) ? (inicio_leitura +  fatia) : arg->tam_vet;
	int qtd_primos = 0;

	for (int i = inicio_leitura; i < fim_leitura; i++) 
		qtd_primos += eh_primo(arg->vet[i]);
	
	printf("Thread %d -> %d numeros primos!\n", arg->id_pt + 1, qtd_primos);

	return EXIT_SUCCESS;
}

int eh_primo(int num) {
   	for (int cont = 2; cont <= sqrt(num); cont++) 
		if (num % cont == 0)
			return 0;
	return (num == 1) ? 0 : 1;
}

