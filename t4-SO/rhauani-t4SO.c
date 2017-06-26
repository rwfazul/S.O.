// Versão em que cada espectador é simulado por uma thread
/* gcc -o rhauani-t4SO rhauani-t4SO.c -pthread */ 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
 
#define	ESGOTADO                    -1
#define CHANCE_RECUSADO              25    // % chance do pagamento ser recusado	
#define	ESPERA_TRANSACAO             3     // segundos rand transacao aprovada
#define ESPERA_ESCOLHA               3     // segundos rand escolha de lugar 

typedef struct Ingresso {
	int numero;                            // possibilita ingressos comecando em outro valor alem de 1 (default)
	char estado;                           // D = disponivel, R = reservado, X = ocupado
} ingresso;

typedef struct Info {	                   // parametro que cada thread recebe
	int id_espec;
	int qtd_ingressos;
	struct Ingresso *ingressos;
} info;

typedef struct Retorno {                   // retorno de cada thread
	int aprovado;
	int rejeitado;
} retorno;

void testa_parametro(int argc, char const *argv[], int *qtd_ingressos);
struct Ingresso *get_ingressos(int qtd_ingressos);
void *compra_ingresso(void *arg);
int escolhe_lugar(struct Info *param);
int procura_lugar_disponivel(struct Info *param); 
int full(struct Info *param);
int transacao_aprovada(int id_espec);
void imprime_info(struct Ingresso *ingressos, int qtd_ingressos, int qtd_espectadores);
void imprime_situacao(struct Ingresso *ingressos, int qtd_ingressos);
void imprime_resultado(struct Retorno *ret[], struct Ingresso *ingressos, int qtd_espectadores, int qtd_ingressos);

sem_t compra;                   // exclusao mutua na hr da compra dos ingressos
sem_t print;                    // evitar que threads fora secao critica ou em SC diferentes percam cpu na hr do print 
sem_t mutex_lno;                // controla acesso 'lugares_nao_ocupados'
int lugares_nao_ocupados;       // contador lugares nao ocupados (disponiveis ou reservados)

int main(int argc, char const *argv[]) {
	srand( (unsigned)time(NULL) );
	int qtd_ingressos, qtd_espectadores, i;
	sem_init(&compra, 0, 1);    
	sem_init(&print, 0, 1);
	sem_init(&mutex_lno, 0, 1);

	testa_parametro(argc, argv, &qtd_ingressos);

	ingresso *ingressos = get_ingressos(qtd_ingressos);
	lugares_nao_ocupados = qtd_ingressos;   // inicialmente todos lugares disponiveis	
	// qtd_Espectadores de 50% a 150% da qtd_ingressos
	qtd_espectadores = (int) (qtd_ingressos / 2) + rand() % (qtd_ingressos + 1); 
	imprime_info(ingressos, qtd_ingressos, qtd_espectadores);

	pthread_t threads[qtd_espectadores];
	info args[qtd_espectadores];
	retorno *ret[qtd_espectadores];         // estruturas irao ser alocadas na funcao das threads
	for (i = 0; i < qtd_espectadores; i++) {
		args[i].id_espec = i + 1;
		args[i].qtd_ingressos = qtd_ingressos;
		args[i].ingressos = ingressos;
		if ( pthread_create(&threads[i], NULL, compra_ingresso, (void*) &args[i]) ) {
			fprintf(stderr, "Erro ao criar thread %d\n", i);
			exit(-1);
		}
	}

	for (i = 0; i < qtd_espectadores; i++) {
		if ( pthread_join(threads[i], (void*) &ret[i]) != 0 ) { // retorno thread salvo em ret[i]
			fprintf(stderr, "Erro no join da thread %d", i);
			exit(-1);	
		}
	}

	imprime_resultado(ret, ingressos, qtd_espectadores, qtd_ingressos);

	sem_destroy(&compra);      
	sem_destroy(&print);
	sem_destroy(&mutex_lno);
	free(ingressos);
	for (i = 0; i < qtd_espectadores; i++) 
		free(ret[i]);

	return 0;
}

void testa_parametro(int argc, char const *argv[], int *qtd_ingressos) {
	if (argc != 2) {
		fprintf(stderr, "Uso: %s qtd_ingressos\n", argv[0]);
		exit(-1);
	}
	if ( (*qtd_ingressos = atoi(argv[1])) <= 0 ) {
		fprintf(stderr, "%s: qtd_ingressos deve ser > 0\n", argv[0]);
		exit(-1);
	}
}

/* RETORNO: Vetor de ingressos de tamanho qtd_ingressos. Todos disponiveis ('D') e numerados de 1 a qtd_ingressos */
struct Ingresso *get_ingressos(int qtd_ingressos) {
	ingresso *ingressos = (ingresso*) malloc(qtd_ingressos * sizeof(ingresso));
	if (ingressos == NULL) {
		fprintf(stderr, "Memoria insuficiente para alocacao do vetor\n");
		exit(-1);
	}

	for (int i = 0; i < qtd_ingressos; i++) {
		ingressos[i].numero = i + 1;
		ingressos[i].estado = 'D';
	}

	return ingressos;
}

/* RETORNO: struct informando situacao da compra */
void *compra_ingresso(void *arg) {
	info *param = (info*) arg;
	int lugar_escolhido, pagamento_aprovado;
	retorno *ret = (retorno*) malloc(sizeof(retorno));
	ret->aprovado = 0, ret->rejeitado = 0;

	/* Para ter paralelismo na escolha de lugares, colocar o semaforo compra apenas no bloco de codigo da funcao
	 procura_lugar_disponivel */
	sem_wait(&compra);	
	// se todos ocupados 1 OR X = 1, entra no if sem precisar pensar no lugar
	if ( !lugares_nao_ocupados || ((lugar_escolhido = escolhe_lugar(param)) == ESGOTADO) ) {
		sem_wait(&print);     
		printf("Espectador %d nao achou lugar\n", param->id_espec);
		sem_post(&print);
		sem_post(&compra);  // libera para outras theads esperando no wait
		return (void*) ret;
	}

	sem_wait(&print);  // evita printar no meio do print de outra thread que esta fora SC ou em alguma SC diferente
	printf("Espectador %d escolheu o lugar %d\n", param->id_espec, param->ingressos[lugar_escolhido].numero);
	imprime_situacao(param->ingressos, param->qtd_ingressos);
	sem_post(&print);
	sem_post(&compra);

	/* fora SC para n travar outras threads enquanto espera aprovacao */
	pagamento_aprovado = transacao_aprovada(param->id_espec); 

	sem_wait(&print);
	if (pagamento_aprovado) {       // efetua compra efetiva
		ret->aprovado = 1;
		printf("Pagamento espectador %d foi aprovado!\n", param->id_espec);
		param->ingressos[lugar_escolhido].estado = 'X';
		sem_wait(&mutex_lno);
		lugares_nao_ocupados -= 1;  // menos um lugar....
		sem_post(&mutex_lno);
		printf("Lugar %d ocupado\n", param->ingressos[lugar_escolhido].numero);
	}
	else {                          // se pagamento reprovado o lugar volta a ficar disponivel
		ret->rejeitado = 1;
		printf("Pagamento do espectador %d não foi autorizado\n", param->id_espec);
		param->ingressos[lugar_escolhido].estado = 'D';
		printf("Lugar %d voltou a ficar disponivel\n", param->ingressos[lugar_escolhido].numero);
	}
	imprime_situacao(param->ingressos, param->qtd_ingressos);	
	sem_post(&print);

	return (void*) ret;
}

/* RETORNO: Indice do vetor referente ao ingresso disponivel OU -1 em caso de todos lugares ocupados  */
int escolhe_lugar(struct Info *param) {
	int lugar_escolhido;
	sem_wait(&print);
	printf("Espectador %d esta escolhendo lugar...\n", param->id_espec);
	sem_post(&print);

	sleep( random() % ESPERA_ESCOLHA );   // pensando... se pensar d+ uma outra thread pode 'passar na frente'

	while ( (lugar_escolhido = procura_lugar_disponivel(param)) == -1 ) {  // se -1 ainda ha chance de algum lugar estar reservado (podendo voltar a ficar disponivel)
		if (lugares_nao_ocupados == 0)    // verifica se existe possibilidade de algum estar reservado
			return ESGOTADO;
		// Se chegou aqui, quando tentou escolher existia um ou + ingressos reservados e que talvez voltem a ficar disponiveis 
		sleep(1);  // espera para ver se algum lugar ira ficar disponivel por recusa de pagamento...
	}
	
	return lugar_escolhido;
}

/* RETORNO: Indice do vetor referente ao ingresso disponivel OU -1 em caso de nenhum lugar disponivel (pode have reservado) */
int procura_lugar_disponivel(struct Info *param) {
	// sem_wait(&compra);
	int pos_aleatoria = random() % param->qtd_ingressos; // comeca a procura a partir de posicao randomica
	int posicao_escolhida = -1;           // se retornar -1: nenhum ingresso disponivel (pode have ingresso reservado)
	int comp_inicial = 1;                 // comparacao_inicial -> variavel auxiliar para fazer a volta no vetor
	for (int i = pos_aleatoria; i != pos_aleatoria || comp_inicial; i = (i + 1) % param->qtd_ingressos) {
		if (param->ingressos[i].estado == 'D') {
			posicao_escolhida = i;
			param->ingressos[i].estado = 'R';  // se disponivel, reserva
			break;
		}
		comp_inicial = 0;
	}	
	// sem_post(&compra);  -> espectador reservou o lugar escolhido, libera semaforo para outras threads

	return posicao_escolhida;
}

/* RETORNO: 1 = transacao aceita, 0 = transacao recusada */
int transacao_aprovada(int id_espec) {
	sem_wait(&print);
	printf("Espectador %d esta aguardando autorizacao de pagamento...\n", id_espec);
	sem_post(&print);

	sleep(1 + rand() % ESPERA_TRANSACAO);

	return ( (rand() % 100) >= CHANCE_RECUSADO) ? 1 : 0;
}

void imprime_info(struct Ingresso *ingressos, int qtd_ingressos, int qtd_espectadores) {
	printf("\n\t\tBILHETERIA SINCRONIZADA\n");
	printf("- Threads/Espectadores: %d\n- Ingressos: %d\n\n", qtd_espectadores, qtd_ingressos);
	printf("Situacao ingressos: 'D' = disponivel, 'R' = reservado, 'X' = ocupado\n");
	imprime_situacao(ingressos, qtd_ingressos);
	printf("---------------------------------------\n");
}

void imprime_situacao(struct Ingresso *ingressos, int qtd_ingressos) {
	printf("\n");
	for (int i = 0; i < qtd_ingressos; i++) 
		printf("%d[%c] ", ingressos[i].numero, ingressos[i].estado);
	printf("\n\n");
}

void imprime_resultado(struct Retorno *ret[], struct Ingresso *ingressos, int qtd_espectadores, int qtd_ingressos) {
	int aprovados = 0, rejeitados = 0;
	printf("---------------------------------\n");
	printf("\t\tRESULTADO:\n");
	printf("{ Threads/Espectadores: %d, Ingressos: %d }\n", qtd_espectadores, qtd_ingressos);
	imprime_situacao(ingressos, qtd_ingressos);
	printf("---------------------------------\n");
	for (int i = 0; i < qtd_espectadores; i++) {
		aprovados += ret[i]->aprovado;
		rejeitados += ret[i]->rejeitado;
	}
	printf("Total: %d ingressos vendidos\n", aprovados);
	printf("---------------------------------\n");
	if (aprovados == qtd_ingressos)
		printf("- Todos os %d ingressos foram vendidos!\n", qtd_ingressos);
	else
		printf("- Restaram %d ingressos disponiveis!\n", qtd_ingressos - aprovados);

	printf("- %d dos %d espectadores tiveram pagamentos aprovados\n", aprovados, qtd_espectadores);
	printf("- %d dos %d espectadores tiveram pagamentos recusados\n", rejeitados, qtd_espectadores);
	printf("- Espectadores sem ingresso: %d\n", qtd_espectadores - aprovados);
	printf("---------------------------------\n");
}