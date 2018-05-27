### Word Document Index in Distributed Memory Parallel Computing System using MPI

#### COMPILATION
mpicc main.c trie.c linklist.c files.c mpifunc.c -o main

#### RUNNING
mpirun  --allow-run-as-root -np <num-processors> ./main <path>

#### Note: Distributed Memory Model
