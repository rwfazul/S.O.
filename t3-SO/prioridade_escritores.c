/* VERSAO COM PRIORIDADE DOS ESCRITORES:
	- Nessa solução cada leitor tem que requisitar o semaforo tentativa_leitura individualmente.
	- Só o primeiro escritor bloqueia o semaforo tentativa_leitura, assim todos os escritores consecutivos
	podem simplismente usar o recurso assim que for liberado pelo escritor anterior. Apenas o ultimo 
	escritor ira abrir o semaforo para os leitores poderem tentar fazer a leitura.
	- Caso nao existam escritores tentando obter o recurso (indicado pelo sem. tentativa_leitura) os leitores
	fazem a leitura. Quando um escritor chegar ele obtem o recurso o mais cedo possivel, ou seja, espera apenas
	pelo leitor atual acabar a leitura, pois, de outra maneira ele teria que esperar uma fila de leitores inteira
	ate que o ultimo leitor pudesse liberar o semaforo.
	- Os leitores esperam ate o ultimo escritor terminar e liberar o semaforo tentativa_leitura.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define QTD_LEITORES                3
#define QTD_ESCRITORES              2
#define TEMPO_SLEEP_LEITORES        2    // teste: 1
#define TEMPO_SLEEP_ESCRITORES      2											
#define TRUE                        1


sem_t recurso;                  // controla acesso base de dados (recurso), 1 = nenhuma thread escritor/leitor na SC
sem_t mutex_prioridade, tentativa_leitura; 		
sem_t mutex_rc, mutex_wc;       // controla acesso 'rc', 'wc'
int rc, wc;                     // read_count, write_count

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

	sem_init(&recurso, 0, 1);    // segundo parametro: 0 = semaforo compartilhado entre threads de um processo, 1 = entre processos
	sem_init(&mutex_rc, 0, 1);   // terceiro parametro: valor inicializacao
	sem_init(&mutex_wc, 0, 1);
	sem_init(&mutex_prioridade, 0, 1);
	sem_init(&tentativa_leitura, 0, 1);

	rc = 0, wc = 0;

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
	sem_destroy(&mutex_rc);
	sem_destroy(&mutex_wc);
	sem_destroy(&mutex_prioridade);
	sem_destroy(&tentativa_leitura);

	return 0;
}


/*  mutex_prioridade eh necessario para priorizar escritores, sem ele haveria a possibilidade de 
	um escritor e um ou mais leitores estarem simultanemante parados em sem_wait(&tentativa_leitura) 
	esperando o post ser feito por um leitor. Quando isso acontecesse a prioridade do escritor 
	nao poderia ser garantida, ou seja, todos os leitores da fila poderiam executar antes dele.
	O mutex_prioridade garante acesso exclusivo ao bloco de codigo de dentro do semaforo, logo, ate ser
	feito o post de tentativa_leitura, nenhum outro leitor podera ficar parado para competir com algum
	escritor no wait e assim o escritor que estiver esperando o sinal ira executar sem concorrencia.
*/
void *leitor(void *arg) {
	int id = *(int*) arg;
	while (TRUE) {
		// Protocolo de entrada
		sem_wait(&mutex_prioridade);			
		sem_wait(&tentativa_leitura);           // indica que leitor quer entrar
		sem_wait(&mutex_rc);                    // evita condicao de corrida com outros leitores (trava secao de entrada)
		if ( (rc += 1) == 1 )                   // se inscreve como leitor e verifica se foi primeiro
			sem_wait(&recurso);                 // se for o primeiro bloqueia recurso
		sem_post(&mutex_rc);                    // libera secao de entrada para outros leitores
		sem_post(&tentativa_leitura);           // indica que acabou de tentar acessar o recurso
		sem_post(&mutex_prioridade);

		le_base(id);							

		// Protocolo de saida
		sem_wait(&mutex_rc);                    // garante que seja o unico leitor na secao de saida
		if ( (rc -= 1) == 0 )                   // avisa que esta saindo e verifica se eh o ultimo deixando SC
			sem_post(&recurso);                 // se for o ultimo libera recurso
		sem_post(&mutex_rc);                    // libera secao de saida para outros leitores
		usa_dados_lidos(id);
	}
}

void *escritor(void *arg) {
	int id = *(int*) arg;
	while (TRUE) {
		pensa_dados_escrita(id);
		// Protocolo de entrada
		sem_wait(&mutex_wc);                    // reserva secao de entrada
		if ( (wc += 1) == 1 )                   // se reporta como leitor e verifica se foi o primeiro
			sem_wait(&tentativa_leitura);       // nao deixa leitores entrar na SC
		sem_post(&mutex_wc);                    // libera secao de entrada

		sem_wait(&recurso);                     // reserva recurso, impede outros escritores de escrever simultaneamente
		escreve_base(id);                       /* Secao critica recurso */
		sem_post(&recurso);                     // libera recurso

		// Protocolo de saida
		sem_wait(&mutex_wc);                    // reserva secao de saida
		if ( (wc -= 1) == 0 )                   // indica que vai sair e verifica se eh o ultimo escritor
			sem_post(&tentativa_leitura);       // se for o ultimo, libera pros leitores entrar na SC
		sem_post(&mutex_wc);                    // fim secao de saida
	}
}

void le_base(int id) {
	printf("Leitor %d esta lendo dados\n", id);
	sleep( 1 + rand() % TEMPO_SLEEP_LEITORES );
}

void usa_dados_lidos(int id){
	printf("Leitor %d esta usando os dados...\n", id);
	sleep( 1 + rand() % TEMPO_SLEEP_LEITORES );
}

void escreve_base(int id) {					
	printf("Escritor %d esta escrevendo\n", id);
	sleep( 1 + rand() % TEMPO_SLEEP_ESCRITORES );
}

void pensa_dados_escrita(int id) {
	printf("Escritor %d esta pensando...\n", id);
	sleep( 1 + rand() % TEMPO_SLEEP_ESCRITORES );
}	
