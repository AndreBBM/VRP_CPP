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

// Função para ler o arquivo de entrada e retornar um vetor com as demandas
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

// Função para ler o arquivo de entrada e retornar um vetor com as rotas possíveis
vector<vector<int>> LerRotasPossiveis(const string& nomeArquivo, int& numVertices) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo " << nomeArquivo << endl;
        exit(1);
    }
    // le a primeira linha do arquivo e salva o numero de vertices
    arquivo >> numVertices;
     
    // ignora as proximas numVertices-1 linhas
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

struct Economia {
    int i, j;
    double valor;
    Economia(int i, int j, double valor) : i(i), j(j), valor(valor) {}
};

bool compararEconomias(const Economia& a, const Economia& b) {
    return a.valor > b.valor;
}

// Função para encontrar a rota de um nó
int encontrarRota(int node, const vector<vector<int>>& rotas, const vector<vector<int>>& distancias) {
    for (size_t i = 0; i < rotas.size(); ++i) {
        auto it = find(rotas[i].begin(), rotas[i].end(), node);
        if (it != rotas[i].end()) {
            // Verificar se a rota tem custo 0
            for (size_t j = 0; j < rotas[i].size() - 1; ++j) {
                if (distancias[rotas[i][j]][rotas[i][j + 1]] == 0) {
                    return -1; // Rota inválida
                }
            }
            return i;
        }
    }
    return -1;
}

// Função para calcular o custo de uma rota
int calcularCustoRota(const vector<int>& rota, const vector<vector<int>>& distancias) {
    int custo = 0;
    for (size_t i = 0; i < rota.size() - 1; ++i) {
        custo += distancias[rota[i]][rota[i + 1]];
    }
    return custo;
}

// Função para implementar a Heurística de Clarke e Wright
void clarkeWright(const vector<vector<int>>& distancias, int capacidade, const vector<int>& demandas, int maxParadas) {
    int n = distancias.size() - 1; // número de clientes (não inclui depósito)
    
    // Inicializa rotas individuais
    vector<vector<int>> rotas;
    for (int i = 1; i <= n; ++i) {
        rotas.push_back({0, i, 0});
    }

    // Calcula as economias
    vector<Economia> economias;
    for (int i = 1; i <= n; ++i) {
        for (int j = i + 1; j <= n; ++j) {
            int valor = distancias[0][i] + distancias[0][j] - distancias[i][j];
            economias.push_back(Economia(i, j, valor));
        }
    }

    // Ordena as economias em ordem decrescente
    sort(economias.begin(), economias.end(), compararEconomias);

    // Combina rotas com base nas economias
    for (const auto& economia : economias) {
        int i = economia.i;
        int j = economia.j;

        int rotaI = encontrarRota(i, rotas, distancias);
        int rotaJ = encontrarRota(j, rotas, distancias);

        if (rotaI != -1 && rotaJ != -1 && rotaI != rotaJ) {
            int demandaTotal = 0;
            for (int cliente : rotas[rotaI]) {
                if (cliente != 0) {
                    demandaTotal += demandas[cliente];
                }
            }
            for (int cliente : rotas[rotaJ]) {
                if (cliente != 0) {
                    demandaTotal += demandas[cliente];
                }
            }
            
            // Verificar se a combinação excede o número máximo de paradas
            int numParadasRotaI = rotas[rotaI].size() - 2; // Excluindo depósito inicial e final
            int numParadasRotaJ = rotas[rotaJ].size() - 2; // Excluindo depósito inicial e final
            if (numParadasRotaI + numParadasRotaJ > maxParadas) {
                continue; // Pular essa combinação se exceder o limite de paradas
            }

            if (demandaTotal <= capacidade) {
                // Combina as rotas
                vector<int> novaRota = rotas[rotaI];
                novaRota.pop_back();
                novaRota.insert(novaRota.end(), rotas[rotaJ].begin() + 1, rotas[rotaJ].end());
                
                // Verifica se a nova rota é válida
                bool rotaValida = true;
                for (size_t k = 0; k < novaRota.size() - 1; ++k) {
                    if (distancias[novaRota[k]][novaRota[k + 1]] == 0) {
                        rotaValida = false;
                        break;
                    }
                }
                
                if (rotaValida) {
                    rotas[rotaI] = novaRota;
                    rotas.erase(rotas.begin() + rotaJ);
                }
            }
        }
    }

    // Calcula e imprime o custo total
    int custoTotal = 0;
    cout << "Rotas finais:" << endl;
    for (const auto& rota : rotas) {
        for (int cliente : rota) {
            cout << cliente << " ";
        }
        int custoRota = calcularCustoRota(rota, distancias);
        custoTotal += custoRota;
        cout << "(Custo: " << custoRota << ")" << endl;
    }
    cout << "Custo total: " << custoTotal << endl;
}

int main() {
    int numVertices;
    vector<int> demandas = LerDestinoDemanda("grafo.txt", numVertices);
    vector<vector<int>> distancias = LerRotasPossiveis("grafo.txt", numVertices);

    int capacidade = 15; // Capacidade do veículo
    int maxParadas = 5;

    auto start = high_resolution_clock::now();
    clarkeWright(distancias, capacidade, demandas, maxParadas - 2); // -2 pra tirar a saida e entrada
    auto end = high_resolution_clock::now();
    

    auto duration = duration_cast<milliseconds>(end - start).count();
    cout << "Tempo de execução: " << duration << " ms" << endl;

    return 0;
}
