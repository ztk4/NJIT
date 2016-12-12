/* 
   CS288 HW10
*/
#include <stdio.h>
#include <mpi.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#define NELMS 1000000
#define MASTER 0
#define MAXPROCS 16

// Useful for seeing what each region is doing
#ifndef NDEBUG
#define DEBUG_LEN 10000
#define debug_printf(...) fprintf(stderr, __VA_ARGS__);
#define debug_snprintf(...) snprintf(__VA_ARGS__);
#else
#define DEBUG_LEN 0
#define debug_printf(...)
#define debug_snprintf(...) 0
#endif  // NDEBUG

unsigned long dot_product(int, int, int *, int *);
void init_lst(int, int, int *);
void print_lst(int, int, int *, char **, int *);

int main(int argc, char **argv) {
  int i,n,vector_x[NELMS],vector_y[NELMS];
  int sidx,eidx,size;
  unsigned long tmp_prod, prod;  // ul to support large products
  int pid,nprocs, rank;
  double stime,etime;
  int debug_max = DEBUG_LEN, tmp;
  char debug_str[DEBUG_LEN + 1] = "", *debug_start = debug_str;
  int *send_counts = NULL, *displ = NULL;
  div_t d;
  MPI_Status status;

  n = atoi(argv[1]);
  if (n > NELMS) { printf("n=%d > N=%d\n",n,NELMS); exit(1); }

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);

  stime = MPI_Wtime();
  d = div(n, nprocs);
  size = d.quot;
  if (pid == nprocs - 1) size += d.rem;  // Remainder for last proc

  // Only master needs to do initialization of data
  if (pid == MASTER) {
    init_lst(0, n, vector_x);
    init_lst(0, n, vector_y);
    
    // Calculate number to send to each proc, and each proc's displacement
    send_counts = (int *) malloc(nprocs * sizeof(int));
    displ = (int *) malloc(nprocs * sizeof(int));
    for (i = 0; i < nprocs; ++i) {
      send_counts[i] = d.quot;  // n / nprocs; (size)
      displ[i] = i * d.quot;    // i * size
    }
    send_counts[nprocs - 1] += d.rem;  // Last process has to do remainder
  }

  // Use scatterv to send data to all processes
  // First arg    is data buf to send
  // Second arg   is an array of how much to send to proc i
  // Third arg    is an array of the displacement in data buf of proc i's data
  // Fourth arg   is the type of data buf
  // Fifth arg    is the input buffer for recieving data
  // Sixth arg    is the number of elements to recieve
  // Seventh arg  is the type of data to recieve
  // Eighth arg   is the rank of the root proc
  // Ninth arg    is the comm instance
  // NOTE: This is the same as scatter, but allows send counts to change by
  //       process, allowing for cases where nprocs does not divide n
  MPI_Scatterv(vector_x, send_counts, displ, MPI_INT, vector_x, size, MPI_INT,
      MASTER, MPI_COMM_WORLD);
  MPI_Scatterv(vector_y, send_counts, displ, MPI_INT, vector_y, size, MPI_INT,
      MASTER, MPI_COMM_WORLD);

  // Print regions we are taking the dot product of 
  if (debug_max > 0) {
    tmp = debug_snprintf(debug_start, debug_max,
      "pid=%d:\tComputing dot product of [ ", pid);
    debug_start += tmp;
    debug_max -= tmp;
  }
  print_lst(0, size, vector_x, &debug_start, &debug_max);
  if (debug_max > 0) {
    tmp = debug_snprintf(debug_start, debug_max, "] and [ ");
    debug_start += tmp;
    debug_max -= tmp;
  }
  print_lst(0, size, vector_y, &debug_start, &debug_max);
  if (debug_max > 0) {
    debug_snprintf(debug_start, debug_max, "]\n");
  }
  debug_printf("%s", debug_str);
  
  // Compute dot product of region and print it
  tmp_prod = dot_product(0, size, vector_x, vector_y);
  debug_printf("pid=%d:\tDot product of region is %lu\n", pid, tmp_prod);

  // Use reduce with a sum to reduce the tmp_prod's into a single prod
  // First arg    is value to reduce (sent from each process)
  // Second arg   is the reduced value (sent only to root)
  // Third arg    is number of values to send/recv from each process
  // Fourth arg   is the type of the value
  // Fifth arg    is the operator to reduce on
  // Sixth arg    is the rank of the root process
  // Seventh arg  is comm instance
  // NOTE: This call is EQUIVALENT to a call to MPI_Gather into a tmp array,
  //       and then looping over that array and summing each value
  MPI_Reduce(&tmp_prod, &prod, 1, MPI_UNSIGNED_LONG, MPI_SUM, MASTER,
      MPI_COMM_WORLD);

  etime = MPI_Wtime();

  if (pid == MASTER) {
    printf("pid=%d:\tfinal prod=%lu\n",pid,prod);
    printf("pid=%d:\telapsed=%f\n",pid,etime-stime);
    free(send_counts);
    free(displ);
  }
  MPI_Finalize();
}

unsigned long dot_product(int s,int e, int *x, int *y){
  int i;
  unsigned prod = 0;
  for (i = s; i < e; ++i)
    prod += x[i] * y[i];

  return prod;
}

void init_lst(int s, int e, int *l){
  int i;
  for (i=s; i<e; i++) l[i] = i;
}
void print_lst(int s,int e, int *l, char **c, int *max){
  int i, tmp;
  for (i=s; i<e && *max > 0; i++) {
    tmp = debug_snprintf(*c, *max, "%d ", l[i]);
    *c += tmp;
    *max -= tmp;
  }
}

// end of file
