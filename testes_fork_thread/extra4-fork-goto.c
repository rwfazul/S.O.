#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int operacao(int n1, int n2, int (*funcao) (int, int));
int soma(int n1, int n2);
int subtrai(int n1, int n2);
int divide(int n1, int n2);
int multiplica(int n1, int n2);

int main(int argc, char const *argv[]) {

	int n1, n2;
	int again, estado;

	novamente:

	printf("Digite dois numeros: ");
	scanf("%d %d", &n1, &n2);

	printf("\nSoma = %d, Sub = %d, ", operacao(n1, n2, &soma), operacao(n1, n2, &subtrai));
	printf("Div = %d, Mult = %d\n", operacao(n1, n2, &divide), operacao(n1, n2, &multiplica));

	printf("\nNovamente (Sim -> != 0, Nao -> 0): ");
	scanf("%d", &again);

	if (again) {
		// pid_t id_filho = fork();
		if (fork()) {
			printf("\nNao irei fazer nada.\n");
			wait(&estado);
		} else {
			printf("\n\tIrei fazer a operacao novamente -> ");
			goto novamente;
		}

	} else
		printf("\nNovamente = 0.\n");


    printf("\nFim do main.\n");

	return 0;
}

int operacao(int n1, int n2, int (*funcao) (int, int)) {
	return funcao(n1, n2);
}

int soma(int n1, int n2) {
	return n1 + n2;
}

int subtrai(int n1, int n2) {
	return n1 - n2;
}

int divide(int n1, int n2) {
	return (n2 == 0) ? 0 : (n1/n2);
}

int multiplica(int n1, int n2) {
	return n1 * n2;
}