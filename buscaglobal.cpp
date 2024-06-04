#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <climits>
#include <fstream>
#include <set>
#include <limits>
#include <chrono>


using namespace std;
using namespace std::chrono;

// Le o arquivo de entrada e retorna um vetor com as demandas de cada vertice e atualiza o numero de vertices
vector<int> LerDestinoDemanda(const string& nomeArquivo, int& numVertices) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo " << nomeArquivo << endl;
        exit(1);
    }
    
    arquivo >> numVertices;

    vector<int> grafo(numVertices, 0);
    int destino, demanda;
    for (int i = 1; i < numVertices; i++) {
        arquivo >> destino >> demanda;
        grafo[destino] = demanda;
    }
    arquivo.close();
    return grafo;
}

// Le o arquivo de entrada e retorna um vetor com as rotas possiveis e atualiza o numero de vertices
vector<vector<int>> LerRotasPossiveis(const string& nomeArquivo, int& numVertices) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo " << nomeArquivo << endl;
        exit(1);
    }

    arquivo >> numVertices;
     
    // ignora as proximas numVertices-1 linhas (sao as demandas dos vertices lidas anteriormente)
    for (int i = 0; i < numVertices-1; i++) {
        int x, y;
        arquivo >> x >> y;
    }

    // le a quantidade de rotas possiveis
    int numRotas;
    arquivo >> numRotas;

    // cria um grafo com numVertices vertices e inicializa com 0 e le as rotas
    vector<vector<int>> grafo(numVertices, vector<int>(numVertices, 0));

    int origem, destino, custo;
    while (arquivo >> origem >> destino >> custo) {
        grafo[origem][destino] = custo;     // GRAFO DIRECIONADO!
    }
    arquivo.close();
    return grafo;
}

// Função para calcular o custo de uma rota
int calcularCusto(const vector<int>& rota, const vector<vector<int>>& rotas) {
    int custo = 0;
    for (size_t i = 0; i < rota.size() - 1; ++i) {
        int origem = rota[i];
        int destino = rota[i + 1];
        if (rotas[origem][destino] == 0) {
            return -1;  // Indica rota inválida
        }
        custo += rotas[origem][destino];
    }
    return custo;
}

// Função para verificar se a capacidade do veículo é respeitada. A soma das demandas não pode ultrapassar a capacidade
int verificaCapacidade(const vector<int>& rota, const vector<int>& demandas, int capacidadeVeiculo){
    int carga_total = 0;
    for (int index : rota){
        carga_total += demandas[index];
    }
    if (carga_total > capacidadeVeiculo){
        return 0;
    }

    return 1;
}

bool rotaJaExiste(const vector<int>& sub, const vector<vector<int>>& combinacoesValidas) {
    // Verifica se 'sub' já existe em 'combinacoesValidas'
    return find_if(combinacoesValidas.begin(), combinacoesValidas.end(), 
                   [&sub](const vector<int>& existente) {
                       return existente == sub;
                   }) != combinacoesValidas.end();
}

// Função para gerar todas as combinações possíveis de rotas válidas independente do custo
void GerarTodasAsCombinacoesPossiveis(const vector<vector<int>>& rotas, int num_cidades, vector<vector<int>>& resultados, 
                                        vector<int>& demandas, int capacidadeVeiculo) {
    int n = rotas.size();
    vector<int> indices;
    for (int i = 0; i < n; ++i) {
        indices.push_back(i);
    }
    // Gera todas as permutações possíveis
    do {
        // Aqui, você precisa dividir a permutação em várias rotas que respeitem a num_cidades do veículo
        // vamos assumir que cada rota pode ter até 'num_cidades' locais
        for (int i = 0; i < n; i += num_cidades) {
            int j = min(i + num_cidades, n);
            vector<int> sub(indices.begin() + i, indices.begin() + j);
            vector<int> rota_custo;
            // Insere o depósito (assumindo ser o local 0) apenas no início da rota
            sub.insert(sub.begin(), 0);
            sub.push_back(0);
            // Verifica se a rota é válida - rota deve existir, não deve ser repetida e a capacidade do veículo deve ser respeitada
            if (calcularCusto(sub, rotas) != -1 && !rotaJaExiste(sub, resultados) && verificaCapacidade(sub, demandas, capacidadeVeiculo) == 1) {
                resultados.push_back(sub);
            }
        }
    } while (next_permutation(indices.begin() + 1, indices.end()));  // Ignora permutar o depósito
}

// Função para verificar se há sobreposição de nós em todas as rotas
// Cada combinação de rotas deve ter todos os nós e cada nó deve ser visitado apenas uma vez
bool checkAllNodesWithoutOverlap(const vector<vector<int>>& routes, int totalNodes) {
    set<int> visitedNodes;
    for (const auto& route : routes) {
        for (int node : route) {
            // Ignore the depot node (assumed to be 0) if necessary
            if (node == 0) continue;
            // Check if the node has already been visited
            if (visitedNodes.find(node) != visitedNodes.end()) {
                return false; // Node overlap found
            }
            visitedNodes.insert(node);
        }
    }

    // Check if all nodes are visited (ignoring the depot node if necessary)
    return visitedNodes.size() == totalNodes - 1; // Subtract 1 if depot node is excluded
}


// Função recursiva para gerar todas as combinações de rotas
void gerarCombinacoesRecursivo(const vector<vector<int>>& rotas, vector<int>& combinacaoAtual,vector<vector<int>>& todasCombinacoes, 
int tamanho_itinerario, int numVertices, int inicio) {
    // Verifica se a combinação atual contém todas as rotas
    if (combinacaoAtual.size() == tamanho_itinerario) {
        vector<vector<int>> vetor_de_verdade;   // eu crio um vetor de verdade porque aqui so se criam os indices
        for (int local : combinacaoAtual) {
            vetor_de_verdade.push_back(rotas[local]);
        }
        if (checkAllNodesWithoutOverlap(vetor_de_verdade, numVertices)){
            todasCombinacoes.push_back(combinacaoAtual);
        }
        return;
    }

    // Tenta adicionar cada rota que ainda não foi usada
    for (size_t i = inicio; i < rotas.size(); i++) {
        combinacaoAtual.push_back(i);
        gerarCombinacoesRecursivo(rotas, combinacaoAtual, todasCombinacoes, tamanho_itinerario, numVertices, i + 1);
        combinacaoAtual.pop_back();
    }
}

// Função principal para gerar todas as combinações de rotas
vector<vector<int>> gerarTodasAsCombinacoesItinerario(const vector<vector<int>>& rotas, int numVertices) {
    vector<vector<int>> todasCombinacoes;
    vector<int> combinacaoAtual;

    // Esse loop garante que as combinações tenham todos os tamanhos possíveis
    for (int k = 1; k <= rotas.size(); k++){
        gerarCombinacoesRecursivo(rotas, combinacaoAtual, todasCombinacoes, k, numVertices, 0);
    }
    
    return todasCombinacoes;
}

// Função para calcular o custo total de uma combinação de rotas
int calculaCustoTotal(const vector<int>& combinacao, const vector<vector<int>>& rotas_possiveis, const vector<vector<int>>& rotas) {
    int custoTotal = 0;
    for (const auto& indiceRota : combinacao) {
        int custoRota = calcularCusto(rotas_possiveis[indiceRota], rotas);
        if (custoRota == -1) return -1; // Se uma rota é inválida, toda a combinação é inválida
        custoTotal += custoRota;
    }
    return custoTotal;
}

// Função para calcular o menor custo entre todas as combinações de rotas
int calcula_menor_custo_itinerarios(const vector<vector<int>>& todasCombinacoes, const vector<vector<int>>& rotas_possiveis, const vector<vector<int>>& rotas) {
    int menorCusto = numeric_limits<int>::max();
    vector<int> menorCustoIndices;

    for (const auto& combinacao : todasCombinacoes) {   // e.g. combinacao = 3 | 4 | 7 | 8 | 10 | 16 
        int custoTotal = calculaCustoTotal(combinacao, rotas_possiveis, rotas);
        if (custoTotal != -1 && custoTotal < menorCusto) {
            menorCusto = custoTotal;
            menorCustoIndices = combinacao;
        }
    }

    cout << menorCusto << endl;
    for (int index : menorCustoIndices) {
        for (int local : rotas_possiveis[index]) {
            cout << local << " ";
        }
        cout << "| ";
    }

    return menorCusto;
}    

int ResolverVRPComDemanda(vector<vector<int>> locais, vector<int> demandas, int C, int& numVertices){
    int maxParadas = 5;
    vector<vector<int>> rotas_possiveis;

    // Gera todas as combinações possíveis de rotas de todos os tamanhos
    for (int num_cidades = 1; num_cidades < maxParadas-1; num_cidades++){   // Neste caso, "num_cidades" sao quantos nos podem ter em uma rota
        GerarTodasAsCombinacoesPossiveis(locais, num_cidades, rotas_possiveis, demandas, C);  // itera sob todas as possibilidades de rota e adiciona a rotas_possiveis
    }

    vector<vector<int>> todasCombinacoes = gerarTodasAsCombinacoesItinerario(rotas_possiveis, numVertices);
    int menor = calcula_menor_custo_itinerarios(todasCombinacoes, rotas_possiveis, locais); 

    return 0;
}

int main() {
    int C = 15;
    int numVertices;
    vector<int> demandas = LerDestinoDemanda("grafo.txt", numVertices);
    vector<vector<int>> locais = LerRotasPossiveis("grafo.txt", numVertices);
    // Capturar o tempo antes da execução
    auto start = high_resolution_clock::now();
    
    ResolverVRPComDemanda(locais, demandas, C, numVertices);
    
    // Capturar o tempo após a execução
    auto end = high_resolution_clock::now();
    
    // Calcular a duração e exibir
    auto duration = duration_cast<milliseconds>(end - start).count();
    cout << "Tempo de execução: " << duration << " ms" << endl;

    return 0;
}

/*
Parâmetros de entrada:
- C: capacidade do veículo
- nome do arquivo de entrada - neste caso grafo.txt

Funcionamento do programa:
1. Lê o arquivo de entrada e armazena as demandas de cada vértice e as rotas possíveis
2. Gera todas as combinações possíveis de rotas de todos os tamanhos, RESPETANDO A CAPACIDADE DO VEÍCULO E A EXISTÊNCIA DAS ROTAS
e.g. 0 1 0, 0 2 0, 0 3 0, 0 1 2 0, 0 1 3 0, 0 2 3 0, 0 1 2 3 0...
3. Gera todas as combinações possíveis de itinerários, garantindo que todos os nós sejam visitados e que não haja sobreposição de nós
e.g. itinerário 1: 0 1 2 0 | 0 3 0; itinerário 2: 0 1 3 0 | 0 2 0...
4. Calcula o custo total de cada combinação de itinerários e exibe a combinação com o menor custo
5. Exibe o tempo de execução do programa

PS: O por ser um algorítmo força bruto ele só funciona para um número pequeno de vértices, testando na VM com 10 vértices o tempo de execução foi de 15s
*/