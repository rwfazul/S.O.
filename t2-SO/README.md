# t2-SO


Trabalho que consiste na comparação de desempenho da busca por números primos Sequencial vs Processos vs Threads.

Compilar:
	<b>gcc -o <nome_arquivo> <nome_arquivo>.c -pthread -lm</b>
	
Uso:
	<b>./<nome_arquivo> tam_vet qtd_pt</b>

(Sendo tam_vet e qtd_pt (quantidade de processos e threads a serem criados) valores positivos e não nulos e tam_vet >= qtd_pt).

-----------------------------------------------------------------
<b>Exemplo de output (tam_vet = 20, qtd_pt = 4):</b><br/><br/>

124 357 452 123 152 360 206 498 368 13 448 430 147 186 487 479 326 297 365 329 

Sequencial -> 3 numeros primos!
Thread 1 -> 0 numeros primos!
Thread 2 -> 1 numeros primos!
Thread 3 -> 1 numeros primos!
Thread 4 -> 1 numeros primos!
Processo 4336 -> 0 numeros primos!
Processo 4337 -> 1 numeros primos!
Processo 4338 -> 1 numeros primos!
Processo 4339 -> 1 numeros primos!

  Tempo execucao busca sequencial:    0.000038s
	Tempo execucao busca com threads:   0.001671s
	Tempo execucao busca com processos: 0.002305s
