#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS			 	10

typedef struct T_args {
	int id_thread;
	int qtd_threads;
} t_args;

void *printHello(void *arg);

int main(int argc, char const *argv[]) {
	pthread_t threads[NUM_THREADS];
	int i;
	t_args *arg;

	for (i = 0; i < NUM_THREADS; i++) {
		if ( (arg = (t_args*) malloc(sizeof(t_args))) == NULL ) {
			fprintf(stderr, "Erro ao alocar mem thread %d\n", i);
			exit(-1);
		}
		arg->id_thread = i;
		arg->qtd_threads = NUM_THREADS;
		printf("Criando a thread %d\n", i);
		if ( pthread_create(&threads[i], NULL, printHello, (void*) arg) ) {
			fprintf(stderr, "Erro ao criar a thread %d\n", i);
			exit(-1);			
		}
	}

	for (i = 0; i < NUM_THREADS; i++) {
		if ( pthread_join(threads[i], NULL) ) {
			fprintf(stderr, "Erro ao executar join %d\n", i);
			exit(-1);				
		}
	}


	return 0;
}

void *printHello(void *arg) {
	t_args *args = (t_args*) arg;
	printf("Hello World, sou a thread %d de %d threads\n", args->id_thread, args->qtd_threads);

	free(arg);
	pthread_exit(NULL);
}