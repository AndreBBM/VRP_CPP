#!/bin/bash
#SBATCH --job-name=mpi_job           # Nome do job
#SBATCH --output=mpi_output_%j.txt   # Nome do arquivo de saída (%j será substituído pelo ID do job)
#SBATCH --error=mpi_error_%j.txt     # Nome do arquivo de erro (%j será substituído pelo ID do job)
#SBATCH --nodes=2                    # Número de nós
#SBATCH --ntasks=4                   # Número de tarefas (processos MPI)
#SBATCH --cpus-per-task=1            # Número de CPUs por tarefa
#SBATCH --time=00:10:00              # Tempo máximo de execução (HH:MM:SS)
#SBATCH --partition=normal           # Partição

# Carregar módulo MPI
module load mpi

# Compilar o programa
mpic++ -o MPI MPI.cpp 

# Executar o programa
mpirun -np 4 ./MPI
