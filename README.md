# VRP_CPP

## Descrição do Projeto

Este projeto tem como objetivo desenvolver e implementar diferentes algoritmos em C++ para resolver o Problema de Roteamento de Veículos (VRP). O VRP é um problema clássico de otimização combinatória que busca determinar a rota ótima para um conjunto de veículos que devem atender a um conjunto de clientes, minimizando o custo total de transporte e respeitando restrições de capacidade dos veículos, quantidade máxima de veículos, existência de rotas, quantidade máxima de paradas por veículo e ponto de partida e chegada dos veículos.

### Os algoritmos implementados neste projeto incluem:

- Busca Global (Força Bruta)
- Heurística de Clarke e Wright (Economias)
- Heurística de Inserção Mais Próxima
- Busca Global com paralelização OpenMP
- Busca Global com paralelização OpenMP e MPI

### Estrutura dos Arquivos

- buscaglobal.cpp: Implementa uma abordagem de força bruta para resolver o VRP.
- clarke.cpp: Implementa a heurística de Clarke e Wright.
- greedy.cpp: Implementa a heurística de Inserção Mais Próxima.
- openmp.cpp: Implementa a busca global paralelizada com OpenMP.
- MPI.cpp: Implementa a busca global paralelizada com OpenMP e MPI.

### Requisitos
Compilador C++ 
Biblioteca OpenMP para paralelização com OpenMP
Biblioteca MPI (OpenMPI ou MPICH) para paralelização com MPI
Arquivo de entrada (grafo.txt) contendo os dados do problema (gerado pelo geraGrafo.py)

### Gerando o arquivo de entrada

Para gerar o arquivo de entrada, execute o script geraGrafo.py:
```sh
python geraGrafo.py
```
você pode querer mudar o número de nós (num_nos) de entrada para testar as heurísticas. O valor inicial de 10 é razoável para testes das implementações de busca global. Para testar as heurísticas, valores maiores (100 a 200) são recomendados.

### Compilando e Executando

Para compilar os arquivos buscaglobal.cpp, clarke.cpp e greedy.cpp execute o seguinte comando:
```sh
g++ -o buscaglobal buscaglobal.cpp
g++ -o clarke clarke.cpp
g++ -o greedy greedy.cpp
```

Para compilar o arquivo openmp.cpp execute o seguinte comando:
```sh
g++ -fopenmp -o openmp openmp.cpp
```

Para compilar o arquivo MPI.cpp execute o seguinte comando:
```sh
mpic++ -o MPI MPI.cpp
```

Para executar os arquivos compilados, execute os seguintes comandos, respectivamente:
```sh
./buscaglobal
./clarke
./greedy
./openmp
mpirun -np <num_processes> ./MPI
```
### Informações Adicionais

Para mais informações sobre o projeto, consulte o arquivo de relatório pdf.
