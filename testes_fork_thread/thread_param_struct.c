#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void *printHello(void *arg);


struct Argumentos {
	int num_th;
	int *vet;
};

#define NUM_THREADS  	5

int main(int argc, char const *argv[]) {
	pthread_t threads[NUM_THREADS];
 	struct Argumentos args[NUM_THREADS];
	int vet[] = {100, 200, 300, 400, 500};
	for (int i = 0; i < NUM_THREADS; i++) {
		args[i].num_th = i;
		args[i].vet = vet;
		printf("Main: Criando thread %d\n", i);
		pthread_create(&threads[i], NULL, printHello, (void*) &args[i]);
	}

	for (int i = 0; i < NUM_THREADS; i++)
		pthread_join(threads[i], NULL);

	return 0;
}

void *printHello(void *arg) {
	struct Argumentos args = *(struct Argumentos*) arg;
	int num_th = args.num_th;
	printf("Hello world! Sou a thread %d. Li o vet e achei o valor %d.\n", num_th, args.vet[num_th]);
}



