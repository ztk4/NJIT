// Zachary Kaplan
// ztk4 (31353570)
// CS288
// 10/26/16

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>

#define OK 1
#define NOK 0
#define NELM 100		/* default 100 elements */
#define NMAX 100000000 /*1e8*/

// min and max for rand float generation
#define FMIN -1000
#define FMAX 1000

void merge_sort(int *, int *, unsigned long int);
void msort_recursive(int *, int *, unsigned long int, unsigned long int);

void selection_sort(int *, unsigned long int);
void radix_sort(unsigned int *, unsigned int *, unsigned long int, int);
void self_check(int *, unsigned long int);

void init_lst(int *, unsigned long int);
void print_lst(int *, unsigned long int);

void init_flst(float *, unsigned long int);
void print_flst(float *, unsigned long int);
void self_checkf(float *, unsigned long int);

int lst[NMAX], tmp[NMAX];
float flst[NMAX], ftmp[NMAX];

int main(int argc,char **argv) {

  long int del_sec,del_msec;
  struct timeval tv_s,tv_e;

  long unsigned int n;
  if (argc>1) n = strtoul(argv[1], NULL, 0);
  else n = NELM;
  printf("n=%lu\n",n);
#if defined(SELECTION) || defined(MERGE) || defined(INT_RADIX)
  init_lst(lst,n);
  //print_lst(lst,n);
#endif  // defined(SELECTION) || defined(MERGE) || defined(INT_RADIX)
#ifdef FLOAT_RADIX
  init_flst(flst, n);
  //print_flst(flst, n);
#endif  // FLOAT_RADIX

  gettimeofday(&tv_s, NULL); 
#ifdef SELECTION
  selection_sort(lst,n);
#endif  // SELECTION

#ifdef MERGE
  merge_sort(lst,tmp,n);
#endif  // MERGE

#ifdef INT_RADIX
  // 1 implies underlying type (int) is signed
  radix_sort((unsigned int *)lst,(unsigned int *)tmp,n, 1);
#endif  // INT_RADIX

#ifdef FLOAT_RADIX
  // 1 implies underlying type (float) is signed
  radix_sort((unsigned int *)flst,(unsigned int *)ftmp,n, 1);
#endif   // FLOAT_RADIX

  gettimeofday(&tv_e, NULL); 

  time_t sdiff = tv_e.tv_sec - tv_s.tv_sec;
  suseconds_t usdiff = tv_e.tv_usec - tv_s.tv_usec;
  int num_char;

  printf("%.6f%n seconds\n", sdiff + usdiff/1e6, &num_char);
  printf("%*.3f milliseconds\n%*ld microseconds\n",
      num_char,
      sdiff * 1000 + usdiff/1e3,
      num_char,
      sdiff * 1000000 + usdiff); 

#if defined(SELECTION) || defined(MERGE) || defined(INT_RADIX)
  //print_lst(lst,n);
  self_check(lst, n);
#endif  // defined(SELECTION) || defined(MERGE) || defined(INT_RADIX)
#ifdef FLOAT_RADIX
  //print_flst(flst, n);
  self_checkf(flst, n);
#endif  // FLOAT_RADIX

  return 0;
}

void selection_sort(int list[], long unsigned int n){
  int min, *minp, *head, *ip, *e = list + n;
  for (head = list; head < e; ++head) {
    min = *head;
    minp = head;
    for (ip = head + 1; ip < e; ++ip) {
      if (*ip < min) {
        min = *ip;
        minp = ip;
      }
    }

    *minp = *head;
    *head = min;
  }
}

void merge_sort(int list[], int temp[], long unsigned int n){
  memcpy(temp, list, sizeof(int) * n);
  msort_recursive(list, temp, 0, n);
}

//use recursion
//left inclusive
//right exclusive
void msort_recursive(int list[], int temp[], long unsigned int left,
    long unsigned int right){
  if (right - left < 2) {
    return;
  }

  // divide...
  // switch temp and list to assure there's always a place to copy to
  long unsigned int mid = (left + right) / 2;
  msort_recursive(temp, list, left, mid);
  msort_recursive(temp, list, mid, right);
  
  // now temp is sorted from left to mid
  // and sorted from mid to right
  // merge in list

  // and conquer
  // e1 end of lower partition in temp
  // e2 end of upper partition in temp
  // lower to be used to iterate over lower partition
  // upper to be used to iterate over upper partition
  // to is place in list to copy to in list
  int *e1 = temp + mid, *e2 = temp + right, *lower, *upper, *to;
  for (lower = temp + left, upper = e1, to = list + left;
      lower < e1 && upper < e2;  // merge both partitions until one is exhausted
      ++to) {
    *to = *lower < *upper ? *lower++ : *upper++;  //always copy the min first
  }
  
  // finish copying over whichever partition is not exhausted
  for(; lower < e1; *to++ = *lower++);
  for(; upper < e2; *to++ = *upper++);
}

// Does a single pass of radix sort
// Result will be in dst, source is in src
// n is the size of both arrays
// i is the pass number [0, 4)
// int sign is truthy if this pass should be sorted as if leftmost bit is a sign
// bit
void radix_pass(unsigned int *dst, unsigned int *src, long unsigned int n,
    int i, int sign) {
  int group = 8;  // 8 bits at a time
  int numb = 1 << group;  // number of buckets
  // buckets[byte] = starting point of region for that byte
  // buckets[byte + 1] = just past end of region for that byte
  long unsigned int *buckets = (long unsigned int *)
    calloc(numb + 1, sizeof(long unsigned int));  // 0 initialized
  //end pointers
  unsigned int *de = dst + n, *se = src + n;
  long unsigned int *be = buckets + numb + 1;

  // Compute count of each byte in buckets
  for (unsigned int *s = src; s < se; ++s) {
    // buckets[B + 1] is count of byte B (this makes indexing easier later)
    ++buckets[((*s >> i*group) & 0xFF) + 1];
  }

  // Compute running sum of the counts in buckets
  for (long unsigned int *b = buckets + 1; b < be - 1; ++b) {
    b[1] += *b;  // Add this bucket into next bucket
  }

  // Update offsets in buckets if this is a signed pass
  //  - by default, negative values are sorted larger than positive values
  //    and by magnitude (so backwards)
  if (sign) {
    // num_neg = N - num_pos; buckets[x] = number of values in buckets before x
    // therefore buckets[numb >> 1] is number of values in first half of buckets
    long unsigned int num_neg = n - buckets[numb >> 1];
    // Shift all offsets of positive numbers (first half) up by num_neg
    for (long unsigned int *b = buckets; b < buckets + (numb >> 1);
        *b++ += num_neg);
    // Invert all negative offsets to point to end of region, and to be before
    // all positive numbers (negative numbers need to be filled in backwards,
    // so end of region is useful because offset can be decremented).
    for (long unsigned int *b = buckets + (numb >> 1); b < be - 1; ++b) {
      // Invert about n (n - *b) and account for 0-indexing (-1)
      *b = n - *b - 1;
    }
  }

  // Fill dst in with values stably sorted by byte i
  for (unsigned int *s = src; s < se; ++s) {
    if (sign && (*s & (1 << 31))) {  // If signed pass and first bit is 1 (neg)
      // Then insert at offset and decrement for next value
      dst[buckets[(*s >> i*group) & 0xFF]--] = *s;
    } else {  // Either not signed pass, or first bit is 0 (pos)
      // Then insert at offset and increment for next value
      dst[buckets[(*s >> i*group) & 0xFF]++] = *s;
    }
  }

  free(buckets);
}

//fix the bucket size to 256. run 4 passes where each pass processes 8 bits
//use lst and tmp only. do not use any more memory of size n.
//sign is truthy if underlying input is signed
void radix_sort(unsigned int *list, unsigned int *temp, long unsigned int n,
    int sign) {
  // From list -> temp; 0th pass; not a signed pass
  radix_pass(temp, list, n, 0, 0);
  // From temp -> list; 1st pass; not a signed pass
  radix_pass(list, temp, n, 1, 0);
  // From list -> temp; 2nd pass; not a signed pass
  radix_pass(temp, list, n, 2, 0);
  // From temp -> list; 3rd pass; signed pass if underlying input is signed
  radix_pass(list, temp, n, 3, sign);
}

void print_lst(int *l, long unsigned int n){
  long unsigned int i;
  for (i=0; i<n; i++) {
    printf("%d ",l[i]);
  }
  printf("\n");
}

void init_lst(int *l, long unsigned int n){
  long unsigned int i;
  //  int seed = time(0) % 100;	/* seconds since 1/1/1970 */
  //  printf ("seed=%d\n", seed);
  srand(1234);			/* SEED */
  for (i=0; i<n; i++) {
    l[i] = rand();
  }
}

void self_check(int *listp, long unsigned int n) {
  long unsigned int i,j,flag=OK;

  for (i=0;i<n-1;i++)
     if (listp[i] > listp[i+1]) { flag = NOK; break; }

  if (flag == OK) printf("sorted\n");
  else printf("NOT sorted at %lu\n",i);
}

void print_flst(float *f, long unsigned int n) {
  for(float *v = f; v < f + n; ++v)
    printf("%f ",  *v);
  printf("\n");
}

void init_flst(float *f, long unsigned int n) {
  srand(1234);
  for (float *v = f; v < f + n; ++v) {
    *v = (float) rand() / ((float) RAND_MAX / (FMAX - FMIN)) + FMIN;
  }
}

void self_checkf(float *f, long unsigned int n) {
  for (float *v = f + 1; v < f + n; ++v) {
    if (*v < *(v-1)) {
      printf("NOT sorted at %lu\n", v - f - 1);
      return;
    }
  }

  printf("sorted\n");
}
