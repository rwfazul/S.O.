/*  gcc -o call_center call_center.c -pthread -Wall */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct Args {
	int id;
	int nro_linhas;
} args;

int valida_argumentos(int argc, char const *argv[]);
void *atendente_worker(void *arg);
void *cliente_worker(void *arg);
void antende_cliente();
void cliente_ligando(int id_cliente);
void recebe_atendimento(int id_cliente);
void desiste_atendimento(int id_cliente);

sem_t clientes;                 // número de clientes esperando atendimento 
sem_t atendentes;               // número de atendentes esperando clientes 
sem_t mutex;                    // evita race conditions em 'waiting' 
int waiting = 0;                // clientes que estao esperando (nao estão sendo atendidos)

#define NUM_ATENDENTES          1     // default = 1
#define TRUE                    1

int main(int argc, char const *argv[]) {
    int nro_linhas = valida_argumentos(argc, argv); // numero de linhas telefonica para os clientes em espera
    pthread_t atendente, cliente;
    int count_clientes = 0;
    args *arg;

    sem_init(&clientes, 0, 0);
    sem_init(&atendentes, 0, 0);
    sem_init(&mutex, 0, 1);

  	/* criando atendente(s) - no caso apenas um */
  	for (int i = 0; i < NUM_ATENDENTES; i++) {
		if ( pthread_create(&atendente, NULL, atendente_worker, NULL) ) {
			fprintf(stderr, "Erro ao criar thread %d", i);
			exit(-1);		
		}
	}

    /* criação indefinida de clientes */
    while(TRUE) {
    	arg = (args*) malloc(sizeof(args));
    	arg->id = ++count_clientes;
    	arg->nro_linhas = nro_linhas;
		if ( pthread_create(&cliente, NULL, cliente_worker, (void*) arg) ) {
			fprintf(stderr, "Erro ao criar thread %d", count_clientes);
			exit(-1);		
		}
		sleep(1);
    }

    sem_destroy(&clientes);
    sem_destroy(&atendentes);
    sem_destroy(&mutex);

    return 0;
}

int valida_argumentos(int argc, char const *argv[]) {
	int nro_linhas;
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <nro_linhas>\n", argv[0]);
		exit(-1);
    } 
    if ( (nro_linhas = atoi(argv[argc-1])) <= 0 ) {
		fprintf(stderr, "%s: numero de linhas deve ser > 0\n", argv[0]);
		exit(-1);
    } 
    return nro_linhas;
}

void* atendente_worker(void *arg){
    while(TRUE) {
        sem_wait(&clientes);            // vai tomar cafe se o número de clientes for 0 
        sem_wait(&mutex);               // obtém acesso a 'waiting' 
        waiting = waiting - 1;          // decrementa o contador de clientes à espera 
        sem_post(&atendentes);          // o antendente esta pronto para falar no telefone
        sem_post(&mutex);               // libera 'waiting'
        antende_cliente();              // antende o telefone (fora da regiao critica)
    }

    pthread_exit(EXIT_SUCCESS);
}

void* cliente_worker(void *arg){
    args *info = (args*) arg;

    sem_wait(&mutex);                   // entra na região crítica 
    if(waiting < info->nro_linhas) {    // se todas as linhas estiverem ocupadas, saia
        cliente_ligando(info->id);
        waiting = waiting + 1;          // incrementa o contador de clientes esperando
        sem_post(&clientes);            // "chama" o atendente, se necessário 
        sem_post(&mutex);               // libera o acesso a 'waiting' 
        sem_wait(&atendentes);          // espera se o atendente estiver no telefone
        recebe_atendimento(info->id);   // falando no telefone (fora da regiao critica)
    }
    else{
        sem_post(&mutex);               // todas as linhas estao ocupadas, nao espera
        desiste_atendimento(info->id);  // desiste do antendimento (fora da regiao critica)
    }

    free(info);
    pthread_exit(EXIT_SUCCESS);
}

void antende_cliente() {
    printf("Atendente esta falando com algum cliente! Ha %d chamadas em espera\n", waiting);
    sleep(3);
}

void cliente_ligando(int id) {
    printf("Cliente %d esta ligando! Havia %d chamadas em espera\n", id, waiting);
}

void recebe_atendimento(int id) {
    printf("Cliente %d esta sendo atendido! Ha %d chamadas em espera\n", id, waiting);
}

void desiste_atendimento(int id) {
    printf("Cliente %d não consegue realizar a chamada. Todas as linhas ocupadas\n", id);
}
