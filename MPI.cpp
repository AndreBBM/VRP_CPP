#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <set>
#include <limits>
#include <chrono>
#include <numeric>
#include <mpi.h>

using namespace std;
using namespace std::chrono;

// Funções de leitura dos arquivos e cálculo de custos (mantidas sem alteração)

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

vector<vector<int>> LerRotasPossiveis(const string& nomeArquivo, int& numVertices) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo " << nomeArquivo << endl;
        exit(1);
    }

    arquivo >> numVertices;
     
    for (int i = 0; i < numVertices-1; i++) {
        int x, y;
        arquivo >> x >> y;
    }

    int numRotas;
    arquivo >> numRotas;

    vector<vector<int>> grafo(numVertices, vector<int>(numVertices, 0));

    int origem, destino, custo;
    while (arquivo >> origem >> destino >> custo) {
        grafo[origem][destino] = custo;
    }
    arquivo.close();
    return grafo;
}

int calcularCusto(const vector<int>& rota, const vector<vector<int>>& rotas) {
    int custo = 0;
    for (size_t i = 0; i < rota.size() - 1; ++i) {
        int origem = rota[i];
        int destino = rota[i + 1];
        if (rotas[origem][destino] == 0) {
            return -1;
        }
        custo += rotas[origem][destino];
    }
    return custo;
}

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
    return find(combinacoesValidas.begin(), combinacoesValidas.end(), sub) != combinacoesValidas.end();
}

// Função para gerar todas as combinações possíveis de rotas válidas independente do custo
void GerarTodasAsCombinacoesPossiveis(const vector<vector<int>>& rotas, int num_cidades, vector<vector<int>>& resultados, 
                                      vector<int>& demandas, int capacidadeVeiculo) {
    int n = rotas.size();
    vector<int> indices(n);
    iota(indices.begin(), indices.end(), 0);

    vector<vector<int>> all_permutations;
    do {
        all_permutations.push_back(indices);
    } while (next_permutation(indices.begin() + 1, indices.end()));

    #pragma omp parallel for schedule(dynamic)
    for (size_t k = 0; k < all_permutations.size(); ++k) {
        const vector<int>& perm = all_permutations[k];
        for (int i = 0; i < n; i += num_cidades) {
            int j = min(i + num_cidades, n);
            vector<int> sub(perm.begin() + i, perm.begin() + j);
            vector<int> rota_custo;
            sub.insert(sub.begin(), 0);
            sub.push_back(0);

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

    sort(resultados.begin(), resultados.end());
    resultados.erase(unique(resultados.begin(), resultados.end()), resultados.end());
}

bool checkAllNodesWithoutOverlap(const vector<vector<int>>& routes, int totalNodes) {
    set<int> visitedNodes;
    for (const auto& route : routes) {
        for (int node : route) {
            if (node == 0) continue;
            if (visitedNodes.find(node) != visitedNodes.end()) {
                return false;
            }
            visitedNodes.insert(node);
        }
    }
    return visitedNodes.size() == totalNodes - 1;
}

void gerarCombinacoesRecursivo(const vector<vector<int>>& rotas, vector<int>& combinacaoAtual, vector<vector<int>>& todasCombinacoes, 
                               int tamanho_itinerario, int numVertices, int inicio) {
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

    for (size_t i = inicio; i < rotas.size(); ++i) {
        combinacaoAtual.push_back(i);
        gerarCombinacoesRecursivo(rotas, combinacaoAtual, todasCombinacoes, tamanho_itinerario, numVertices, i + 1);
        combinacaoAtual.pop_back();
    }
}

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

    sort(todasCombinacoes.begin(), todasCombinacoes.end());
    todasCombinacoes.erase(unique(todasCombinacoes.begin(), todasCombinacoes.end()), todasCombinacoes.end());

    return todasCombinacoes;
}

int calculaCustoTotal(const vector<int>& combinacao, const vector<vector<int>>& rotas_possiveis, const vector<vector<int>>& rotas) {
    int custoTotal = 0;
    for (const auto& indiceRota : combinacao) {
        int custoRota = calcularCusto(rotas_possiveis[indiceRota], rotas);
        if (custoRota == -1) return -1;
        custoTotal += custoRota;
    }
    return custoTotal;
}

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

int ResolverVRPComDemanda(vector<vector<int>> locais, vector<int> demandas, int C, int& numVertices, int rank, int size) {
    int maxParadas = 5;
    vector<vector<int>> rotas_possiveis;

    for (int num_cidades = 1; num_cidades < maxParadas - 1; ++num_cidades) {
        GerarTodasAsCombinacoesPossiveis(locais, num_cidades, rotas_possiveis, demandas, C);
    }

    vector<vector<int>> todasCombinacoes = gerarTodasAsCombinacoesItinerario(rotas_possiveis, numVertices);

    int menorCustoLocal = numeric_limits<int>::max();
    vector<int> menorCustoIndices;

    #pragma omp parallel for
    for (size_t i = rank; i < todasCombinacoes.size(); i += size) {
        const auto& combinacao = todasCombinacoes[i];
        int custoTotal = calculaCustoTotal(combinacao, rotas_possiveis, locais);
        if (custoTotal != -1 && custoTotal < menorCustoLocal) {
            #pragma omp critical
            {
                if (custoTotal < menorCustoLocal) {
                    menorCustoLocal = custoTotal;
                    menorCustoIndices = combinacao;
                }
            }
        }
    }

    int menorCustoGlobal;
    MPI_Reduce(&menorCustoLocal, &menorCustoGlobal, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "Menor custo: " << menorCustoGlobal << endl;
    }

    return menorCustoGlobal;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int C = 15;
    int numVertices;
    vector<int> demandas = LerDestinoDemanda("grafo.txt", numVertices);
    vector<vector<int>> locais = LerRotasPossiveis("grafo.txt", numVertices);

    auto start = high_resolution_clock::now();
    
    int resultado = ResolverVRPComDemanda(locais, demandas, C, numVertices, rank, size);
    
    auto end = high_resolution_clock::now();
    
    if (rank == 0) {
        auto duration = duration_cast<milliseconds>(end - start).count();
        cout << "Tempo de execução: " << duration << " ms" << endl;
    }

    MPI_Finalize();
    return 0;
}
