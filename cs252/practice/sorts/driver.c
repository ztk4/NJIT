#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


extern void ssort(int *arr, int N);
extern void isort(int *arr, int N);
/*
extern void hsort(int *arr, int N);
extern void msort(int *arr, int N);
extern void qsort(int *arr, int N);
*/

extern void rand_arr(int *arr, int N);
extern void print_arr(int *arr, int N);

int main(int argc, char **argv) {
    int N, *arr, *sorted;
    srand(time(NULL));

    printf("Please enter an array size: ");
    scanf("%d", &N);
    arr = (int *) malloc(N * sizeof(int));
    sorted = (int *) malloc(N * sizeof(int));

    rand_arr(arr, N);
    
    printf("\nUnsorted Array: ");
    print_arr(arr, N);

    memcpy(sorted, arr, N * sizeof(int));
    printf("\nSelection Sort: ");
    ssort(sorted, N);
    print_arr(sorted, N);
    
    memcpy(sorted, arr, N * sizeof(int));
    printf("\nInsertion Sort: ");
    isort(sorted, N);
    print_arr(sorted, N);

    memcpy(sorted, arr, N * sizeof(int));
    printf("\nHeap Sort: ");
    hsort(sorted, N);
    print_arr(sorted, N);

    free(arr);
    free(sorted);

    return 0;
}
