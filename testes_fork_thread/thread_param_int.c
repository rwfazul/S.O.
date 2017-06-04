#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS			10

void *printHello(void *arg);

int main(int argc, char const *argv[]) {
	pthread_t threads[NUM_THREADS];
	int i;
	int *arg;

	for (i = 0; i < NUM_THREADS; i++) {
		if ( (arg = (int*) malloc(sizeof(int))) == NULL ) {
			fprintf(stderr, "Erro ao alocar mem thread %d", i);
			exit(-1);
		}
		*arg = i;
		printf("Criando a thread %d\n", i);
		if ( pthread_create(&threads[i], NULL, printHello, (void*) arg) ) {
			fprintf(stderr, "Erro ao criar a thread %d", i);
			exit(-1);			
		}
	}

	for (i = 0; i < NUM_THREADS; i++) {
		if ( pthread_join(threads[i], NULL) ) {
			fprintf(stderr, "Erro ao executar join %d", i);
			exit(-1);				
		}
	}

	return 0;
}

void *printHello(void *arg) {
	int *idThread = (int*) arg;
	printf("Hello World, sou a thread %d\n", *idThread);

	free(arg);
	pthread_exit(NULL);
}