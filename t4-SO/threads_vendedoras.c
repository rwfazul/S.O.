// Versão em que cada thread é um "ponto de venda" que atende mais de um espectador
/* gcc -o rhauani-t4SO rhauani-t4SO.c -pthread */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
 
#define	TRUE                         1
#define NUM_THREADS                  6
#define CHANCE_RECUSADO              25    // % chance do pagamento ser recusado	
#define	ESPERA_TRANSACAO             3     // segundos rand transacao aprovada
#define ESPERA_ESCOLHA               3     // segundos rand escolha de lugar 

typedef struct Ingresso {
	int numero;                            // possibilita ingressos comecando em outro valor alem de 1 (default)
	char estado;                           // D = disponivel, R = reservado, X = ocupado
} ingresso;

typedef struct Info {	                   // parametro que cada thread recebe
	int id_espec;
	int qtd_espectadores;
	int qtd_ingressos;
	struct Ingresso *ingressos;
} info;

typedef struct Retorno {                   // retorno de cada thread
	int qtd_aprovados;
	int qtd_rejeitados;
	int full;                              // todos lugares ocupados
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

sem_t reserva;                  // exclusao mutua na hr de reservar os ingressos
sem_t print;                    // evitar que threads fora secao critica ou em SC diferentes percam cpu na hr do print 
sem_t mutex_ec, mutex_lr;       // controla acesso 'ec', controla acesso 'lr'
int ec, lr;                     // contador de espectadores, contador lugares reservados

int main(int argc, char const *argv[]) {
	srand( (unsigned)time(NULL) );
	pthread_t threads[NUM_THREADS];
	int qtd_ingressos, qtd_espectadores, i;
	info args[NUM_THREADS];
	retorno *ret[NUM_THREADS];  // estruturas irao ser alocadas na funcao das threads

	sem_init(&reserva, 0, 1);    
	sem_init(&print, 0, 1);
	sem_init(&mutex_ec, 0, 1);  
	sem_init(&mutex_lr, 0, 1);
	ec = 0, lr= 0;

	testa_parametro(argc, argv, &qtd_ingressos);
	ingresso *ingressos = get_ingressos(qtd_ingressos);
	qtd_espectadores = (int) (qtd_ingressos / 2) + rand() % (qtd_ingressos + 1); // qtd_Espectadores de 50% a 150% da qtd_ingressos
	imprime_info(ingressos, qtd_ingressos, qtd_espectadores);

	for (i = 0; i < NUM_THREADS; i++) {
		args[i].qtd_espectadores = qtd_espectadores;
		args[i].qtd_ingressos = qtd_ingressos;
		args[i].ingressos = ingressos;
		if ( pthread_create(&threads[i], NULL, compra_ingresso, (void*) &args[i]) ) {
			fprintf(stderr, "Erro ao criar thread %d\n", i);
			exit(-1);
		}
	}

	for (i = 0; i < NUM_THREADS; i++) {
		if ( pthread_join(threads[i], (void*) &ret[i]) != 0 ) { // retorno thread salvo em ret[i]
			fprintf(stderr, "Erro no join da thread %d", i);
			exit(-1);	
		}
	}

	imprime_resultado(ret, ingressos, qtd_espectadores, qtd_ingressos);

	sem_destroy(&reserva);    
	sem_destroy(&print);
	sem_destroy(&mutex_ec);
	sem_destroy(&mutex_lr);
	free(ingressos);
	for (i = 0; i < NUM_THREADS; i++) 
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

/* RETORNO: numero de pagamentos aceitos e recusados e motivo de saida (todos ingressos vendidos ou fim dos espectadores) */
void *compra_ingresso(void *arg) {
	info *param = (info*) arg;
	int lugar_escolhido, pagamento_aprovado;
	retorno *ret = (retorno*) malloc(sizeof(retorno));
	ret->qtd_aprovados = 0, ret->qtd_rejeitados = 0, ret->full = 0;

	while (TRUE) {
		sem_wait(&mutex_ec);             // evita condicao de corrida com novos espectadores
		if ( (ec += 1) > param->qtd_espectadores ) {   // se inscreve como espectador e verifica se eh o ultimo
			sem_post(&mutex_ec);         // se for o ultimo libera semaforo e retorna
			return (void*) ret;
		}
		param->id_espec = ec;            // salva id do espectador atual da thread
		sem_post(&mutex_ec);             // libera secao de entrada para outros espectadores

		//sem_wait(&reserva); -> se usar aqui nao permite pararelismo de espectadores pensando no lugar
		lugar_escolhido = escolhe_lugar(param);
		while (lugar_escolhido == -1) {   // se -1 ainda ha chance de algum lugar estar reservado (podendo voltar a ficar disponivel)
			// if ( full(param->vet) ) -> varredura desnecessaria
			if ( lr == 0 ) {              // verifica se existe algum que nao esteja ocupado
				ret->full = 1;            // todos ingressos vendidos
				sem_wait(&print);     
				printf("Espectador %d nao achou lugar\n", param->id_espec);
				sem_post(&print);
				return (void*) ret;
			}
			// Se chegou aqui, quando tentou escolher existia um ou + ingressos reservados que talvez voltem a ficar disponivel (pagamento nao aceito)
			sleep(1);   // espera para ver se algum lugar ira ficar disponivel por recusa de pagamento...
			lugar_escolhido = procura_lugar_disponivel(param);  // nao precisa passar pelo rand de espera novamente
		}
		sem_wait(&print);  // evita printar no meio do print de outra thread que esta fora SC ou em alguma SC diferente
		printf("Espectador %d escolheu o lugar %d\n", param->id_espec, param->ingressos[lugar_escolhido].numero);
		imprime_situacao(param->ingressos, param->qtd_ingressos);
		sem_post(&print);

		/* fora SC para n travar outras threads enquanto espera aprovacao */
		pagamento_aprovado = transacao_aprovada(param->id_espec); 


		//sem_wait(&compra);  -> desnecessario
		sem_wait(&print);
		if (pagamento_aprovado) {       // efetua compra efetiva
			ret->qtd_aprovados = ret->qtd_aprovados + 1;
			printf("Pagamento espectador %d foi aprovado!\n", param->id_espec);
			param->ingressos[lugar_escolhido].estado = 'X';
			printf("Lugar %d ocupado\n", param->ingressos[lugar_escolhido].numero);
		}
		else {                          // se pagamento reprovado o lugar volta a ficar disponivel
			ret->qtd_rejeitados = ret->qtd_rejeitados + 1;
			printf("Pagamento do espectador %d não foi autorizado\n", param->id_espec);
			param->ingressos[lugar_escolhido].estado = 'D';
			printf("Lugar %d voltou a ficar disponivel\n", param->ingressos[lugar_escolhido].numero);
		}
		imprime_situacao(param->ingressos, param->qtd_ingressos);
		sem_wait(&mutex_lr);               
		lr -= 1;                        // menos um ingresso reservado (foi ocupado ou liberado)
		sem_post(&mutex_lr);		
		sem_post(&print);
		/* Espectador acabou sua compra (com sucesso ou nao), thread ira atender outro espectador (se existir) */
	}
}

/* RETORNO: Indice do vetor referente ao ingresso disponivel OU -1 em caso de nenhum ingresso disponivel no momento */
int escolhe_lugar(struct Info *param) {
	sem_wait(&print);
	printf("Espectador %d esta escolhendo lugar...\n", param->id_espec);
	sem_post(&print);

	sleep( random() % ESPERA_ESCOLHA );   // pensando... se pensar d+ uma outra thread pode 'passar na frente'
	
	return procura_lugar_disponivel(param);
}

/* Quando chamada na funcao compra_ingresso diretamente evita o espectador de 'pensar' novamente no lugar */
int procura_lugar_disponivel(struct Info *param) {
	sem_wait(&reserva);                   // garante exclusao mutua para efetuar a reserva efetiva
	int pos_aleatoria = random() % param->qtd_ingressos;
	int posicao_escolhida = -1;           // se retornar -1: nenhum ingresso disponivel (pode have ingresso reservado)
	int comp_inicial = 1;                 // comparacao_inicial -> variavel auxiliar para fazer a volta no vetor
	for (int i = pos_aleatoria; i != pos_aleatoria || comp_inicial; i = (i + 1) % param->qtd_ingressos) {
		if (param->ingressos[i].estado == 'D') {
			posicao_escolhida = i;
			param->ingressos[i].estado = 'R';  // se disponivel, reserva
			sem_wait(&mutex_lr);          
			lr += 1;                           // mais um lugar ocupado
			sem_post(&mutex_lr);
			break;
		}
		comp_inicial = 0;
	}
	sem_post(&reserva);                   // espectador reservou o lugar escolhido, libera semaforo para outras threads
	
	return posicao_escolhida;
}

/* RETORNO: 1 = todos ingressos vendidos, 0 = existe algum disponivel ou resevado */
int full(struct Info *param) {
	for (int i = 0; i < param->qtd_ingressos; i++) {
		if (param->ingressos[i].estado != 'X')
			return 0;
	}
	sem_wait(&print);     
	printf("Espectador %d nao achou lugar\n", param->id_espec);
	sem_post(&print);

	return 1;
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
	printf("- Threads: %d\n- Ingressos: %d\n- Espectadores: %d\n\n", NUM_THREADS, qtd_ingressos, qtd_espectadores);
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
	int aprovados = 0, rejeitados = 0, full = 0;
	printf("---------------------------------\n");
	printf("\t\tRESULTADO:\n");
	printf("{ Threads: %d, Ingressos: %d, Espectadores: %d }\n", NUM_THREADS, qtd_ingressos, qtd_espectadores);
	imprime_situacao(ingressos, qtd_ingressos);
	printf("---------------------------------\n");
	for (int i = 0; i < NUM_THREADS; i++) {
		printf("Thread %d vendeu %d ingressos\n", i + 1, ret[i]->qtd_aprovados);
		aprovados += ret[i]->qtd_aprovados;
		rejeitados += ret[i]->qtd_rejeitados;
		full += ret[i]->full;
	}
	printf("\nTotal: %d ingressos vendidos\n", aprovados);
	printf("---------------------------------\n");
	if (full || aprovados == qtd_ingressos)
		printf("- Todos os %d ingressos foram vendidos!\n", qtd_ingressos);
	else
		printf("- Restaram %d ingressos disponiveis!\n", qtd_ingressos - aprovados);

	printf("- %d dos %d espectadores tiveram pagamentos aprovados\n", aprovados, qtd_espectadores);
	printf("- %d dos %d espectadores tiveram pagamentos recusados\n", rejeitados, qtd_espectadores);
	printf("- Espectadores sem ingresso: %d\n", qtd_espectadores - aprovados);
	printf("---------------------------------\n");
}