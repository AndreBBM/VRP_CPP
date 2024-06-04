#include <iostream>
#include <vector>
#include <algorithm>
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

// Função para resolver o VRP usando a Heurística de Inserção Mais Próxima
vector<vector<int>> insercaoMaisProxima(const vector<vector<int>>& distancias, const vector<int>& demandas, int capacidade) {
    int n = distancias.size() - 1; // número de clientes (não inclui depósito)
    vector<vector<int>> rotas;
    vector<bool> visitado(n + 1, false);
    visitado[0] = true; // o depósito é sempre visitado

    for (int i = 1; i <= n; ++i) {
        if (visitado[i]) continue;

        vector<int> rota = {0, i};
        visitado[i] = true;
        int cargaAtual = demandas[i];

        while (true) {
            int melhorCliente = -1;
            int menorDistancia = numeric_limits<int>::max();

            for (int j = 1; j <= n; ++j) {
                if (!visitado[j] && cargaAtual + demandas[j] <= capacidade) {
                    int distancia = distancias[rota.back()][j];
                    if (distancia != 0 && distancia < menorDistancia) { // Verifica se a distância não é zero (rota válida)
                        menorDistancia = distancia;
                        melhorCliente = j;
                    }
                }
            }

            if (melhorCliente == -1) break;

            rota.push_back(melhorCliente);
            visitado[melhorCliente] = true;
            cargaAtual += demandas[melhorCliente];
        }

        rota.push_back(0); // retorna ao depósito
        if (calcularCusto(rota, distancias) != -1) { // Verifica se a rota é válida
            rotas.push_back(rota);
        }
    }

    return rotas;
}

// Função para calcular o custo total de uma combinação de rotas
int calcularCustoTotal(const vector<vector<int>>& rotas, const vector<vector<int>>& distancias) {
    int custoTotal = 0;
    for (const auto& rota : rotas) {
        custoTotal += calcularCusto(rota, distancias);
    }
    return custoTotal;
}

int ResolverVRPInsercaoMaisProxima(vector<vector<int>> locais, vector<int> demandas, int capacidade) {
    auto rotas = insercaoMaisProxima(locais, demandas, capacidade);
    int custoTotal = calcularCustoTotal(rotas, locais);

    cout << "Rotas finais:" << endl;
    for (const auto& rota : rotas) {
        for (int cliente : rota) {
            cout << cliente << " ";
        }
        int custoRota = calcularCusto(rota, locais);
        cout << "(Custo: " << custoRota << ")" << endl;
    }
    cout << "Custo total: " << custoTotal << endl;

    return custoTotal;
}

int main() {
    int capacidade = 15;
    int numVertices;
    vector<int> demandas = LerDestinoDemanda("grafo.txt", numVertices);
    vector<vector<int>> locais = LerRotasPossiveis("grafo.txt", numVertices);

    auto start = high_resolution_clock::now();
    
    int resultado = ResolverVRPInsercaoMaisProxima(locais, demandas, capacidade);
    
    auto end = high_resolution_clock::now();
    
    auto duration = duration_cast<milliseconds>(end - start).count();
    cout << "Tempo de execução: " << duration << " ms" << endl;

    return 0;
}
