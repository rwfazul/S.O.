/* VERSAO COM PRIORIDADES IGUAIS
	- Leitores e escritores vao receber acesso ao recurso em funcao da ordem de chegada,
	se um escritor chegar enquanto leitores estao acessando o recurso, ele ira esperar ate
	esses leitores liberarem o recurso e depois ira realizar sua escrita. Se novos leitores
	chegarem nesse meio tempo, eles irao esperar.
	- Para realizar essa "fila justa" entre leitores e escritores (prevenir starvation)
	eh necessario usar um semaforo para enfileirar as requisicoes. Qualquer thread que queira o
	recurso devera passar por esse semaforo e libera-lo assim que ganhar acesso ao recurso em si.
	- Esse semaforo devera perservar a ordem FIFO ao bloquear e liberar as threads, de outra maneira
	um escritor bloqueado, por exemplo, poderia permanecer bloqueado indefinidamente por conta de 
	um ciclo de outros escritores que conseguiriam decrementar o semaforo antes dele proprio.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define QTD_LEITORES                    3
#define QTD_ESCRITORES                  2
#define TEMPO_SLEEP_LEITORES            3	
#define TEMPO_SLEEP_ESCRITORES          3									
#define TRUE                            1

sem_t fila;                     // turno, ordem de chegada
sem_t recurso;                  // controla acesso base de dados (recurso), 1 = nenhuma thread escritor/leitor na SC
sem_t mutex_rc;                 // controla acesso 'rc'
int rc;                         // numero de processos lendo ou querendo ler (read_count)

void *leitor(void *arg);
void *escritor(void *arg);
void le_base(int id);
void usa_dados_lidos(int id);
void escreve_base(int id);
void pensa_dados_escrita(int id);

int main(int argc, char const *argv[]) {
	srand( (unsigned) time(NULL) );
	int i, leitores_id[QTD_LEITORES], escritores_id[QTD_LEITORES];
	pthread_t leitores[QTD_LEITORES], escritores[QTD_ESCRITORES];

	sem_init(&fila, 0, 1);
	sem_init(&recurso, 0, 1);   // segundo parametro: 0 = semaforo compartilhado entre threads de um processo, 1 = entre processos
	sem_init(&mutex_rc, 0, 1);  // terceiro parametro: valor inicializacao
	int rc = 0;

	for (i = 0; i < QTD_LEITORES; i++) {
		leitores_id[i] = i + 1;
		if ( pthread_create(&leitores[i], NULL, leitor, (void*) &leitores_id[i]) ) {
			fprintf(stderr, "Erro ao criar thread leitor %d\n", i);
			exit(-1);
		}
	}
	for (i = 0; i < QTD_ESCRITORES; i++) {
		escritores_id[i] = i + 1;
		if ( pthread_create(&escritores[i], NULL, escritor, (void*) &escritores_id[i]) ) {
			fprintf(stderr, "Erro ao criar thread escritor %d\n", i);
			exit(-1);
		}
	}

	for (i = 0; i < QTD_LEITORES; i++)
		pthread_join(leitores[i], NULL);
	for (i = 0; i < QTD_ESCRITORES; i++)
		pthread_join(escritores[i], NULL);

	sem_destroy(&fila);
	sem_destroy(&recurso);
	sem_destroy(&mutex_rc);

	return 0;
}

void *leitor(void *arg) {
	int id = *(int*) arg;
	while (TRUE) {
		// Protocolo Entrada
		sem_wait(&fila);               // ordem de chegada
		sem_wait(&mutex_rc);           // garante acesso exclusivo 'rc': nenhum outro leitor pode entrar SC enquanto essa thread n sair
		if ( (rc += 1) == 1 )          // indica que esta tentando entrar SC e verifica se novo leitor eh o primeiro a tentar entrar SC
			sem_wait(&recurso);        // se for o primeiro bloqueia recurso para os escritores

		sem_post(&fila);               // thread ja foi atendida (obteve recurso)
		sem_post(&mutex_rc);           // libera acesso 'rc'

		le_base(id);				

		// Protocolo saida
		sem_wait(&mutex_rc);            // acesso exclusivo 'rc': nenhum outro leitor pode sair da SC enquanto essa thread n sair
		if ( (rc -= 1) == 0 )           // indica que nao vai mais usar recurso e verifica se eh o ultimo
			sem_post(&recurso);         // se for ultimo libera recurso -> disponivel para os escritores 
		sem_post(&mutex_rc);            // libera acesso 'rc'
		usa_dados_lidos(id);			
	}
}

void *escritor(void *arg) {
	int id = *(int*) arg;
	while (TRUE) {
		pensa_dados_escrita(id);
		sem_wait(&fila);                // ordem de chegada
		sem_wait(&recurso);             // requisita acesso exclusivo ao recurso
		sem_post(&fila);                // libera semaforo de ordem de chegada, essa thread ja foi atendida

		escreve_base(id);               /* Secao critica */	

		sem_post(&recurso);             // libera recurso
	}
}

// Teste utilizando sleep com tempo fixo para ver melhor os resultados
void le_base(int id) {
	printf("Leitor %d esta lendo dados\n", id);
	sleep( rand() % TEMPO_SLEEP_LEITORES );
	// sleep(1);
}

void usa_dados_lidos(int id){
	printf("Leitor %d esta usando os dados...\n", id);
	sleep( rand() % TEMPO_SLEEP_LEITORES );
	// sleep(1);
}

void escreve_base(int id) {					
	printf("Escritor %d esta escrevendo\n", id);
	sleep( rand() % TEMPO_SLEEP_ESCRITORES );	
	// sleep(1);
}

void pensa_dados_escrita(int id) {
	printf("Escritor %d esta pensando...\n", id);
	sleep( rand() % TEMPO_SLEEP_ESCRITORES );
	// sleep(1);
}	
