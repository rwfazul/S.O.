# t3-SO

ELC1080 Disciplina de Fundamentos de Banco de Dados | 4° semestre do curso de Sistemas de Informação UFSM 2017-1

Trabalho que consiste na implementação e testes das três soluções do problema dos leitores-escritores (readesr-writers problem), em que:

- Processos leitores e escritores competem pelo acesso a uma base de dados;
- Vários leitores podem acessar a base ao mesmo tempo;
- Quando um escritor está na base de dados, nenhum outro processo pode acessá-la (nem mesmo um leitor).


### 1. Prioridade dos leitores 
Também chamado: **First readers-writers problem**, **readers-preference**, **starving writers**, **favoring readers**, **reader-priority**.
– Sempre que um leitor quiser ler e não houver escritor escrevendo (pode haver escritor esperando), ele tem acesso à base. Nesta solução, um escritor pode ter que esperar indefinidamente (starvation), pois novos leitores sempre chegam.

###2. Prioridade dos escritores
Também chamado: **Second readers-writers problem**, **writers-preference**, **starving readers**, **favoring writers**, **writer-priority**.
– Quando um escritor desejar escrever nenhum leitor pode ler enquanto o escritor não for atendido. Nesta solução, um leitor pode ter de esperar indefinidamente (starvation), pois novos escritores sempre chegam.

####3. Prioridades iguais
Também chamado: **Third readers-writers problem**, **no-starvation solution**, **fair solution**.
– Não há risco de starvation, pois leitores e escritores têm as mesmas chances de acesso à base; pode haver uma queda de desempenho em relação às soluções anteriores.
