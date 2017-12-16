#include <stdio.h>
#include <stdlib.h>

typedef int (*teste)(int paramUm, int paramDois);

int testeUm(int paramUm, int paramDois);
int testeDois(int paramUm, int paramDois);
int testeTres(int paramUm, int paramDois);
int testeQuatro(int paramUm, int paramDois);

teste testes[] = {
	testeUm,
	testeDois,
	testeTres,
	testeQuatro
};

#define MAX_TESTES			4

int main(int argc, char const *argv[]) {
	int param1 = 1, param2 = 0;

    for (int i = 0; i < MAX_TESTES; i++) {
        if (testes[i](param1, param2)) {
            printf("Teste %d = sucesso!\n", i+1);
        } else {
            printf("Teste %d = falha!\n", i+1);
        }
    } 

	return 0;
}

int testeUm(int paramUm, int paramDois) {
	return paramUm;
}

int testeDois(int paramUm, int paramDois) {
	return paramDois;
}

int testeTres(int paramUm, int paramDois) {
	return paramUm;
}

int testeQuatro(int paramUm, int paramDois) {
	return paramDois;
}
