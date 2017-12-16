/* gcc -o rhauani-t2SO rhauani-t2SO.c -pthread -lm */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>


void busca_thread(int num);
void *par(void *info);
void *impar(void *info);

typedef struct Parametro {
	int id_pt;                             // identificação thread ou processo (pid)
	int *vet;
	int indice_inicio; 
	int indice_fim;
} parametro;

typedef struct Retorno {
	int id_pt;
	int qtd_primos;
} retorno;


int main(int argc, char const *argv[]) {
	
	busca_thread(160);

	return 0;
}

void busca_thread(int num) {

	parametro info;
	info.id_pt = num;
	pthread_t threads;
	if (num % 2) { // impar
		if ( pthread_create(&threads, NULL, impar, (void*) &info) ) {
			fprintf(stderr, "Erro ao criar thread");
			exit(-1);		
		}		
	} else { // par
		if ( pthread_create(&threads, NULL, par, (void*) &info) ) {
			fprintf(stderr, "Erro ao criar thread");
			exit(-1);		
		}	
	}

	
	retorno *ret;
	/* Processo principal ("thread pai") imprime resultados de todas threads criadas a partir dele */
		if ( pthread_join(threads, (void*) &ret) == 0 ) {      // retorno da funcao 
			printf("%d ", ret->id_pt);
			int num_aux = ret->id_pt;
			free(ret);
			if (num_aux != 1)
				busca_thread(num_aux);
		}
		else {
			fprintf(stderr, "Erro no join da thread");
			exit(-1);	
		}
	
}


void *par(void *info) {
	parametro *param = (parametro*) info;

	retorno *ret= (retorno*) malloc(sizeof(retorno));
	ret->id_pt = param->id_pt/2;

 	return ((void*) ret);
}

void *impar(void *info) {
	parametro *param = (parametro*) info;

	retorno *ret= (retorno*) malloc(sizeof(retorno));
	ret->id_pt = (param->id_pt*3)+1;

 	return ((void*) ret);
}
