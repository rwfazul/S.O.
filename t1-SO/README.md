# t1-SO


Trabalho que consiste na comparação de desempenho da criação de processos em cadeia vs árvore binária cheia

Compilar: 
	gcc -o <nome_arquivo> <nome_arquivo>.c
	
Uso:
	./<nome_arquivo> altura

(Sendo altura um valor positivo não nulo.)

Exemplo de output (altura = 2):

	Cadeia (inicio em PID = 6465)

Possuo PID=6465 e PPID=6440

Possuo PID=6466 e PPID=6465

Possuo PID=6467 e PPID=6466

Possuo PID=6468 e PPID=6467

Possuo PID=6469 e PPID=6468

Possuo PID=6470 e PPID=6469

Possuo PID=6471 e PPID=6470

Termino do processo com PID=6471 e PPID=6470
Termino do processo com PID=6470 e PPID=6469
Termino do processo com PID=6469 e PPID=6468
Termino do processo com PID=6468 e PPID=6467
Termino do processo com PID=6467 e PPID=6466
Termino do processo com PID=6466 e PPID=6465
Termino execucao cadeia, PID=6465


Tempo execucao processos em cadeia: 0.003048s

-------------------------------------------------
	Arvore (inicio em PID = 6465)
	
Possuo PID=6465 e PPID=6440
Possuo PID=6472 e PPID=6465
Possuo PID=6473 e PPID=6465
Possuo PID=6477 e PPID=6473
Termino do processo com PID=6477 e PPID=6473
Possuo PID=6474 e PPID=6472
Termino do processo com PID=6474 e PPID=6472
Possuo PID=6475 e PPID=6473
Termino do processo com PID=6475 e PPID=6473
Termino do processo com PID=6473 e PPID=6465
Possuo PID=6476 e PPID=6472
Termino do processo com PID=6476 e PPID=6472
Termino do processo com PID=6472 e PPID=6465
Termino execucao arvore, PID=6465


Tempo execucao processos em arvore: 0.001849s
