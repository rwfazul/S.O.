# t1-SO


Trabalho que consiste na comparação de desempenho da criação de processos em cadeia vs árvore binária cheia

Compilar:</br>
	<b>gcc -o <nome_arquivo> <nome_arquivo>.c</b>
	
Uso:</br>
	<b>./<nome_arquivo> altura</b>

(Sendo altura um valor positivo não nulo.)

Exemplo de output (altura = 2):<br/><br/>

	Cadeia (inicio em PID = 6465)

Possuo PID=6465 e PPID=6440<br/>
Possuo PID=6466 e PPID=6465<br/>
Possuo PID=6467 e PPID=6466<br/>
Possuo PID=6468 e PPID=6467<br/>
Possuo PID=6469 e PPID=6468<br/>
Possuo PID=6470 e PPID=6469<br/>
Possuo PID=6471 e PPID=6470<br/>
Termino do processo com PID=6471 e PPID=6470<br/>
Termino do processo com PID=6470 e PPID=6469<br/>
Termino do processo com PID=6469 e PPID=6468<br/>
Termino do processo com PID=6468 e PPID=6467<br/>
Termino do processo com PID=6467 e PPID=6466<br/>
Termino do processo com PID=6466 e PPID=6465<br/>
Termino execucao cadeia, PID=6465<br/><br/>


Tempo execucao processos em cadeia: 0.003048s<br/><br/>

-------------------------------------------------
	Arvore (inicio em PID = 6465)
	
Possuo PID=6465 e PPID=6440<br/>
Possuo PID=6472 e PPID=6465<br/>
Possuo PID=6473 e PPID=6465<br/>
Possuo PID=6477 e PPID=6473<br/>
Termino do processo com PID=6477 e PPID=6473<br/>
Possuo PID=6474 e PPID=6472<br/>
Termino do processo com PID=6474 e PPID=6472<br/>
Possuo PID=6475 e PPID=6473<br/>
Termino do processo com PID=6475 e PPID=6473<br/>
Termino do processo com PID=6473 e PPID=6465<br/>
Possuo PID=6476 e PPID=6472<br/>
Termino do processo com PID=6476 e PPID=6472<br/>
Termino do processo com PID=6472 e PPID=6465<br/>
Termino execucao arvore, PID=6465<br/>


Tempo execucao processos em arvore: 0.001849s
