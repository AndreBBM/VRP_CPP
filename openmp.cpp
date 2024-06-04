#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <set>
#include <limits>
#include <chrono>
#include <omp.h>
#include <numeric>  // Inclui a biblioteca necessária para usar iota


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
    return find(combinacoesValidas.begin(), combinacoesValidas.end(), sub) != combinacoesValidas.end();
}

// Função para gerar todas as combinações possíveis de rotas válidas independente do custo
void GerarTodasAsCombinacoesPossiveis(const vector<vector<int>>& rotas, int num_cidades, vector<vector<int>>& resultados, 
                                      vector<int>& demandas, int capacidadeVeiculo) {
    int n = rotas.size();
    vector<int> indices(n);
    iota(indices.begin(), indices.end(), 0);

    vector<vector<int>> all_permutations;
    // Gera todas as permutações possíveis
    do {
        all_permutations.push_back(indices);
    } while (next_permutation(indices.begin() + 1, indices.end()));  // Ignora permutar o depósito

    // Paraleliza a avaliação das permutações
    #pragma omp parallel for schedule(dynamic)
    for (size_t k = 0; k < all_permutations.size(); ++k) {
        const vector<int>& perm = all_permutations[k];
        for (int i = 0; i < n; i += num_cidades) {
            int j = min(i + num_cidades, n);
            vector<int> sub(perm.begin() + i, perm.begin() + j);
            vector<int> rota_custo;
            // Insere o depósito (assumindo ser o local 0) apenas no início da rota
            sub.insert(sub.begin(), 0);
            sub.push_back(0);

            // Verifica se a rota é válida - rota deve existir, não deve ser repetida e a capacidade do veículo deve ser respeitada
            if (calcularCusto(sub, rotas) != -1 && verificaCapacidade(sub, demandas, capacidadeVeiculo)) {
                #pragma omp critical
                {
                    if (!rotaJaExiste(sub, resultados)) {
                        resultados.push_back(sub);
                    }
                }
            }
        }
    }

    // Remove duplicatas dos resultados finais
    sort(resultados.begin(), resultados.end());
    resultados.erase(unique(resultados.begin(), resultados.end()), resultados.end());
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
void gerarCombinacoesRecursivo(const vector<vector<int>>& rotas, vector<int>& combinacaoAtual, vector<vector<int>>& todasCombinacoes, 
                               int tamanho_itinerario, int numVertices, int inicio) {
    // Verifica se a combinação atual contém todas as rotas
    if (combinacaoAtual.size() == tamanho_itinerario) {
        vector<vector<int>> vetor_de_verdade;
        for (int local : combinacaoAtual) {
            vetor_de_verdade.push_back(rotas[local]);
        }
        if (checkAllNodesWithoutOverlap(vetor_de_verdade, numVertices)) {
            #pragma omp critical
            todasCombinacoes.push_back(combinacaoAtual);
        }
        return;
    }

    // Tenta adicionar cada rota que ainda não foi usada
    for (size_t i = inicio; i < rotas.size(); ++i) {
        combinacaoAtual.push_back(i);
        gerarCombinacoesRecursivo(rotas, combinacaoAtual, todasCombinacoes, tamanho_itinerario, numVertices, i + 1);
        combinacaoAtual.pop_back();
    }
}

// Função principal para gerar todas as combinações de rotas
vector<vector<int>> gerarTodasAsCombinacoesItinerario(const vector<vector<int>>& rotas, int numVertices) {
    vector<vector<int>> todasCombinacoes;
    
    #pragma omp parallel
    {
        vector<vector<int>> todasCombinacoesLocal;
        vector<int> combinacaoAtual;

        #pragma omp for schedule(dynamic)
        for (int k = 1; k <= rotas.size(); ++k) {
            gerarCombinacoesRecursivo(rotas, combinacaoAtual, todasCombinacoesLocal, k, numVertices, 0);
        }

        #pragma omp critical
        {
            todasCombinacoes.insert(todasCombinacoes.end(), todasCombinacoesLocal.begin(), todasCombinacoesLocal.end());
        }
    }

    // Remover duplicatas das combinações finais
    sort(todasCombinacoes.begin(), todasCombinacoes.end());
    todasCombinacoes.erase(unique(todasCombinacoes.begin(), todasCombinacoes.end()), todasCombinacoes.end());

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

    #pragma omp parallel for
    for (size_t i = 0; i < todasCombinacoes.size(); ++i) {
        const auto& combinacao = todasCombinacoes[i];
        int custoTotal = calculaCustoTotal(combinacao, rotas_possiveis, rotas);
        if (custoTotal != -1 && custoTotal < menorCusto) {
            #pragma omp critical
            {
                if (custoTotal < menorCusto) {
                    menorCusto = custoTotal;
                    menorCustoIndices = combinacao;
                }
            }
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
    for (int num_cidades = 1; num_cidades < maxParadas - 1; ++num_cidades) {  // Neste caso, "num_cidades" são quantos nós podem ter em uma rota
        GerarTodasAsCombinacoesPossiveis(locais, num_cidades, rotas_possiveis, demandas, C);  // Itera sob todas as possibilidades de rota e adiciona a rotas_possiveis
    }

    vector<vector<int>> todasCombinacoes = gerarTodasAsCombinacoesItinerario(rotas_possiveis, numVertices);
    int menor = calcula_menor_custo_itinerarios(todasCombinacoes, rotas_possiveis, locais); 

    return menor;
}

int main() {
    int C = 15;
    int numVertices;
    vector<int> demandas = LerDestinoDemanda("grafo.txt", numVertices);
    vector<vector<int>> locais = LerRotasPossiveis("grafo.txt", numVertices);
    
    // Capturar o tempo antes da execução
    auto start = high_resolution_clock::now();
    
    int resultado = ResolverVRPComDemanda(locais, demandas, C, numVertices);
    
    // Capturar o tempo após a execução
    auto end = high_resolution_clock::now();
    
    // Calcular a duração e exibir
    auto duration = duration_cast<milliseconds>(end - start).count();
    cout << "\nMenor custo: " << resultado << "\n";
    cout << "Tempo de execução: " << duration << " ms" << endl;

    return 0;
}
