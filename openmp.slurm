#!/bin/bash
#SBATCH --job-name=openmp_job        # Nome do job
#SBATCH --output=openmp_output_%j.txt # Nome do arquivo de saída (%j será substituído pelo ID do job)
#SBATCH --error=openmp_error_%j.txt   # Nome do arquivo de erro (%j será substituído pelo ID do job)
#SBATCH --nodes=1                    # Número de nós
#SBATCH --ntasks=1                   # Número de tarefas (processos MPI)
#SBATCH --cpus-per-task=4            # Número de CPUs por tarefa
#SBATCH --time=00:10:00              # Tempo máximo de execução (HH:MM:SS)
#SBATCH --partition=normal           # Partição
#SBATCH --mem=4GB                    # Quantidade de memória por nó (ajuste conforme necessário)

# Compilar o programa
g++ -fopenmp -o openmp openmp.cpp 

# Executar o programa
./openmp
