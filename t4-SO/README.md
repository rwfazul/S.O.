# t4-SO

ELC1080 Disciplina de Sistemas Operacionais A | 4° semestre do curso de Sistemas de Informação UFSM 2017-1


### Bilheteria Sincronizada <br/>
Trabalho que simula a sincronização da venda de bilhetes em uma bilheteria.

#### Tipos <br/>
- rhauani-t4SO.c
	+ Versão em que cada thread simula um espectador, não há paralelismo na escolha do lugar;
	+ qtd_espectadores de 50% a 150% da quantidade de ingressos; qtd_ingressos passado por parametro.
	
- paralelismo_escolha.c
	+ Versão em que cada thread simula um espectador, há paralelismo no momento em que as threads estão pensando no lugar.
	
- threads_vendedoras.c
	+ Versão em que cada thread é um "ponto de venda" atendendo mais de um espectador;
	+ Número de threads fixos; qtd_espectadores idem rhauani-t4SO; qtd_ingressos passado por parametro.

##### Compilação e execução <br/>
- Compilar:
	**gcc -o <nome_arquivo> <nome_arquivo>.c -pthread**
	
- Uso:
	**./<nome_arquivo> num_espectadores <br/>**

##### Output <br/>
Número de ingressos vendidos e número de espectadores que ficaram sem ingresso.

