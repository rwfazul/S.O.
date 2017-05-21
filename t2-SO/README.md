# t2-SO


Trabalho que consiste na comparação de desempenho da busca por números primos Sequencial vs Processos vs Threads.

Compilar:
	<b>gcc -o <nome_arquivo> <nome_arquivo>.c -pthread -lm</b>
	
Uso:
	<b>./<nome_arquivo> tam_vet qtd_pt</b><br/>

- tam_vet: tamanho do vetor a ser preenchido de forma randômica;<br/>
- qtd_pt: quantidade de processos e threads a serem criados.</br>

(Sendo tam_vet e qtd_pt  valores positivos e não nulos e tam_vet >= qtd_pt).

-----------------------------------------------------------------
<b>Exemplo de output (tam_vet = 20, qtd_pt = 4):</b><br/>

124 357 452 123 152 360 206 498 368 13 448 430 147 186 487 479 326 297 365 329<br/>

Sequencial -> 3 numeros primos!<br/>
Thread 1 -> 0 numeros primos!<br/>
Thread 2 -> 1 numeros primos!<br/>
Thread 3 -> 1 numeros primos!<br/>
Thread 4 -> 1 numeros primos!<br/>
Processo 4336 -> 0 numeros primos!<br/>
Processo 4337 -> 1 numeros primos!<br/>
Processo 4338 -> 1 numeros primos!<br/>
Processo 4339 -> 1 numeros primos!<br/>

	Tempo execucao busca sequencial:    0.000038s
	Tempo execucao busca com threads:   0.001671s
	Tempo execucao busca com processos: 0.002305s
