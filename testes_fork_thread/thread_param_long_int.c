#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void *printHello(void *arg);

#define NUM_THREADS  	5

int main(int argc, char const *argv[]) {
	pthread_t threads[NUM_THREADS];

	for (long i = 0; i < NUM_THREADS; i++) {
		printf("Main: Criando thread %ld\n", i);
		if (pthread_create(&threads[i], NULL, printHello, (void*) i)) {
			fprintf(stderr, "Erro ao criar thread %ld", i);
			exit(-1);		
		}
	}

	for (int i = 0; i < NUM_THREADS; i++)
		pthread_join(threads[i], NULL);

	return 0;
}

void *printHello(void *arg) {
	long a = (long) arg;
	printf("Hello World! Sou a thread %li!\n", a);
	return EXIT_SUCCESS;
}



