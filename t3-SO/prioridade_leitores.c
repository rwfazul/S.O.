/* VERSAO COM PRIORIDADE DOS LEITORES:
	- Leitores diferentes podem usar o mesmo recurso simultaneamente, mas mesmo assim precisam
	de exclusao mutua para executar as sessoes de entrada e saida.
	- Antes de entrar na SC, cada novo leitor garante acesso exclusivo ao contator de leitores (read_count),
	isso evita condicoes de corrida, onde dois leitores incrementam o 'rc' ao mesmo tempo e os dois tentam
	trancar o recurso, causando bloqueio de um leitor. O mesmo eh feito na saida.
	- Nessa solução cada escritor tem que requisitar o recurso individualmente, ou seja, se houver muitos 
	novos leitores chegando ocorerra o bloqueio de potenciais escritores, podendo causar postergação indefinifida.
	- Só o primeiro leitor bloqueia o recurso (se estiver disponivel), assim se o recurso for usado por varios 
	leitores consecutivos nao eh preciso fazer um re-bloqueio. Depois do primeiro leitor travar o recurso, 
	nenhum escritor pode usa-lo antes de ser liberado e o recurso so ira ser liberado pelo ultimo leitor.
	- Quando um escritor termina escrita, se tiverem leitores e escritores esperando, o proximo a executar
	depende do escalonador (normalmente vai selecionar por FIFO).
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define QTD_LEITORES                    3
#define QTD_ESCRITORES                  2
#define TEMPO_SLEEP_LEITORES            3       // teste: 1
#define TEMPO_SLEEP_ESCRITORES          3       // teste: sleep escritores < leitores 						
#define TRUE                            1

sem_t recurso;                  // controla acesso base de dados (recurso), 1 = nenhuma thread escritor/leitor na SC
sem_t mutex;                    // controla acesso 'rc'
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

	sem_init(&recurso, 0, 1);   // segundo parametro: 0 = semaforo compartilhado entre threads de um processo, 1 = entre processos
	sem_init(&mutex, 0, 1);     // terceiro parametro: valor inicializacao
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


	sem_destroy(&recurso);
	sem_destroy(&mutex);

	return 0;
}

void *leitor(void *arg) {
	int id = *(int*) arg;
	while (TRUE) {
		// Protocolo Entrada
		sem_wait(&mutex);              // garante acesso exclusivo 'rc': nenhum outro leitor pode entrar SC enquanto essa thread n sair
		/* Secao critica */            // LEMBRAR: (++rc) == 1 e nao (rc++) == 1 ...
		if ( (rc += 1) == 1 )          // indica que esta tentando entrar SC e verifica se novo leitor eh o primeiro a tentar entrar SC
			sem_wait(&recurso);        // se for o primeiro bloqueia recurso para os escritores
		sem_post(&mutex);              // libera acesso 'rc'

		le_base(id);				

		// Protocolo saida
		sem_wait(&mutex);               // acesso exclusivo 'rc': nenhum outro leitor pode sair da SC enquanto essa thread n sair
		if ( (rc -= 1) == 0 )           // indica que nao vai mais usar recurso e verifica se eh o ultimo
			sem_post(&recurso);         // se for ultimo libera recurso -> disponivel para os escritores 
		sem_post(&mutex);               // libera acesso 'rc'
		// se perder cpu aqui... pode dar resultado invertido? (ex: escritor escreve antes de leitor usar)
		usa_dados_lidos(id);			
	}
}

void *escritor(void *arg) {
	int id = *(int*) arg;
	while (TRUE) {
		pensa_dados_escrita(id);
		sem_wait(&recurso);             // requisita acesso exclusivo ao recurso
		escreve_base(id);               /* Secao critica */				
		sem_post(&recurso);             // libera recurso
	}
}

void le_base(int id) {
	printf("Leitor %d esta lendo dados\n", id);
	sleep( rand() % TEMPO_SLEEP_LEITORES );
}

void usa_dados_lidos(int id){
	printf("Leitor %d esta usando os dados...\n", id);
	sleep( rand() % TEMPO_SLEEP_LEITORES );
}

void escreve_base(int id) {					/* SC */
	printf("Escritor %d esta escrevendo\n", id);
	sleep( rand() % TEMPO_SLEEP_ESCRITORES );	
}

void pensa_dados_escrita(int id) {
	printf("Escritor %d esta pensando...\n", id);
	sleep( rand() % TEMPO_SLEEP_ESCRITORES );
}	
