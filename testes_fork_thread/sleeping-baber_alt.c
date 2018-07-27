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

void valida_argumentos(int argc, char const *argv[], int *nro_atendentes, int *nro_linhas);
void *atendente_worker(void *arg);
void *cliente_worker(void *arg);
void antende_cliente(int id_atendente);
void tira_folga(int id_atendente, int num_atendimentos);
void retorna_trabalho(int id_atendente);
void cliente_ligando(int id_cliente);
void recebe_atendimento(int id_cliente);
void desiste_atendimento(int id_cliente);

sem_t clientes;                 // número de clientes esperando atendimento 
sem_t atendentes;               // número de atendentes esperando clientes 
sem_t mutex;                    // evita race conditions em 'waiting' 
int waiting = 0;                // clientes que estao esperando (nao estão sendo atendidos)

#define TRUE                    1
#define MIN_ATENDIMENTOS_FOLGA  5
#define MIN_TEMPO_FOLGA         2000000
#define MIN_TEMPO_LIGACAO       850000
#define TEMPO_NOVA_lIGACAO      200000
#define MOD_ATENDE              100000
#define MOD_FOLGA               500000
#define MOD_ATENDIMENTOS        3

int main(int argc, char const *argv[]) {
    srand( time(NULL) );
    int nro_linhas, nro_atendentes, count_clientes;
    valida_argumentos(argc, argv, &nro_atendentes, &nro_linhas); // numero de atendentes e linhas telefonicas para os clientes em espera
    pthread_t atendente, cliente;
    args *arg;

    sem_init(&clientes, 0, 0);
    sem_init(&atendentes, 0, nro_atendentes);
    sem_init(&mutex, 0, 1);

    /* criando atendente(s) */
    for (int i = 0; i < nro_atendentes; i++) {
        arg = (args*) malloc(sizeof(args));
        arg->id = i + 1;
        arg->nro_linhas = nro_linhas;
        if ( pthread_create(&atendente, NULL, atendente_worker, (void*) arg) ) {
            fprintf(stderr, "Erro ao criar atendente %d", i);
            exit(-1);		
        }
    }

    /* criação indefinida de clientes */
    count_clientes = 0;
    while(TRUE) {
        arg = (args*) malloc(sizeof(args));
    	arg->id = ++count_clientes;
    	arg->nro_linhas = nro_linhas;
        if ( pthread_create(&cliente, NULL, cliente_worker, (void*) arg) ) {
            fprintf(stderr, "Erro ao criar cliente %d", count_clientes);
            exit(-1);		
        }
        usleep(TEMPO_NOVA_lIGACAO);
    }

    sem_destroy(&clientes);
    sem_destroy(&atendentes);
    sem_destroy(&mutex);

    return 0;
}

void valida_argumentos(int argc, char const *argv[], int *nro_atendentes, int *nro_linhas) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <nro_atendentes> <nro_linhas>\n", argv[0]);
		exit(-1);
    } 
    if ( (*nro_atendentes = atoi(argv[argc-1])) <= 0 ) {
		fprintf(stderr, "%s: numero de atendentes deve ser > 0\n", argv[0]);
		exit(-1);
    } 
    if ( (*nro_linhas = atoi(argv[argc-1])) <= 0 ) {
		fprintf(stderr, "%s: numero de linhas deve ser > 0\n", argv[0]);
		exit(-1);
    } 
}

void* atendente_worker(void *arg){
    args *info = (args*) arg;
    int atendimentos = 0; 
    while(TRUE) {
        if (atendimentos == (info->id * MIN_ATENDIMENTOS_FOLGA) + (rand() % MOD_ATENDIMENTOS)) {
            tira_folga(info->id, atendimentos);
            atendimentos = 0;
            retorna_trabalho(info->id);
        }
        sem_wait(&clientes);            // vai tomar cafe se o número de clientes for 0 
        // if (atendimentos == (info->id * MIN_ATENDIMENTOS_FOLGA) + incr_random) retorna_trabalho(info->id);
        sem_wait(&mutex);               // obtém acesso a 'waiting' 
        waiting = waiting - 1;          // decrementa o contador de clientes à espera 
        sem_post(&atendentes);          // o antendente esta pronto para falar no telefone
        sem_post(&mutex);               // libera 'waiting'
        antende_cliente(info->id);      // antende o telefone (fora da regiao critica)
        atendimentos++;
    }
    
    free(info);
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

void antende_cliente(int id_atendente) {
    printf("Atendente %d esta falando com algum cliente! Ha %d chamadas em espera\n", id_atendente, waiting);
    usleep(MIN_TEMPO_LIGACAO + (rand() % MOD_ATENDE));
}

void tira_folga(int id_atendente, int num_atendimentos) {
    printf("** Atendente %d esta tirando sua folga após %d atendimentos ... ***\n", id_atendente, num_atendimentos);
    usleep(MIN_TEMPO_FOLGA + (rand() % MOD_FOLGA));
}

void retorna_trabalho(int id_atendente) {
    printf("*** Atendente %d esta voltando ao trabalho... ***\n", id_atendente);
}

void cliente_ligando(int id_cliente) {
    printf("Cliente %d esta ligando! Havia %d chamadas em espera\n", id_cliente, waiting);
}

void recebe_atendimento(int id_cliente) {
    printf("Cliente %d esta sendo atendido! Ha %d chamadas em espera\n", id_cliente, waiting);
}

void desiste_atendimento(int id_cliente) {
    printf("Cliente %d não consegue realizar a chamada. Todas as linhas ocupadas\n", id_cliente);
}
