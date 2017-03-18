#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define MASTER 0
#define NELMS 1000
#define MAXPROCS 100

int dot_product(int s, int e, int *x, int *y) {
  int i, prod = 0;
  for (i = s; i < e; ++i)
    prod += x[i] * y[i];

  return prod;
}

void init(int *x, int *y, int n) {
  for (int i = 0; i < n; ++i)
    x[i] = y[i] = i + 1;
}

int main(int argc, char **argv) {
  int n, nprocs, pid, size, prod = 0;
  int vector_x[NELMS], vector_y[NELMS], partial_prods[MAXPROCS];
  MPI_Status status;
  MPI_Comm world;
  n = atoi(argv[1]);

  MPI_Init(&argc, &argv);
  world = MPI_COMM_WORLD;
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);

  if (pid == MASTER)
    init(vector_x, vector_y, n);

  size = n / nprocs;  // must divisible

  MPI_Scatter(vector_x, size, MPI_INT, vector_x, size, MPI_INT, MASTER, world);
  MPI_Scatter(vector_y, size, MPI_INT, vector_y, size, MPI_INT, MASTER, world);

  prod = dot_product(0, size, vector_x, vector_y);

  MPI_Gather(&prod, 1, MPI_INT, partial_prods, 1, MPI_INT, MASTER, world);

  if (pid == MASTER) {
    prod = 0;
    for (int *partial = partial_prods; partial < partial_prods + MAXPROCS;
        ++partial)
      prod += *partial;

    printf("Final product is %d\n", prod);
  }

  MPI_Finalize();
  return 0;
}
