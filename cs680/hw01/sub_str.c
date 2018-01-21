#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* C implementation of sub_str */
static inline char *sub_str(char *dest, char *src, int s_idx, int e_idx) {
  char *pdest = dest, *psrc;
  for (psrc = src + s_idx; psrc < src + e_idx; ++psrc) {
    *pdest++ = *psrc;
  }

  return dest;
}

/* ASM implementation of sub_str */
static inline char *asm_sub_str(char *dest, char *src, int s_idx, int e_idx) {
  char *odest;  // The original destination pointer.
  /* Some of the following is influenced by the answer to my SO question here:
   * https://stackoverflow.com/questions/48362759/gcc-inline-assembly-g-constraint-and-parameter-size
   */
  asm(
      "movq %%rdi, %[odest];   \n\t" /* Save original dest for return */
      "movslq %[s_idx], %%rax; \n\t" /* Sign extend s_idx to 64-bits */
      "addq %%rax, %%rsi;      \n\t" /* Add start index to src */
      "subl %%eax, %%ecx;      \n\t" /* Get length of substr as e - s (0-ext) */
      "rep movsb; \n\t" /* Move %ecx bytes of str at %esi into str at %edi */
      : "+S" (src), "+D" (dest), "+&c" (e_idx), [odest]"=&g" (odest),
        "=m" (*(char (*)[]) dest)  /* Tells compiler we are writing to dest */
      : [s_idx]"g" (s_idx),
        "m" (*(const char (*)[]) src)  /* We are reading from src */
      : "cc", "rax"
      );
  
  return odest;
}

/* Helper for parsing integers w/ error handling */
static int parse_int(char *str) {
  errno = 0;

  long tmp = strtol(str, NULL, 0);
  /* Explicitly check for integer overflow */
  if (!errno && (tmp > INT_MAX || tmp < INT_MIN)) errno = ERANGE;
  /* Handle any error */
  if (errno) {
    fprintf(stderr, "Error parsing integer '%s': %s\n", str, strerror(errno));
    exit(2);
  }

  return tmp;
}

int main(int argc, char **argv) {
  char *src, *dst_c, *dst_asm;
  int s_idx, e_idx;
  size_t len, sub_len;

  if (argc != 4) {
    printf("Usage: %s string s_idx e_idx\n", *argv);
    return 1;
  }

  /* Get inputs from command line */
  src = argv[1];
  s_idx = parse_int(argv[2]);
  e_idx = parse_int(argv[3]);

  /* Validate that substring indices are valid (don't expose the stack!) */
  len = strlen(src);
  if (e_idx < s_idx || e_idx > len || s_idx < 0) {
    fputs("Error: Indices overflow the input string, or the start index "
          "exceeds the end index\n", stderr);
    return 3;
  }

  /* Allocate storage for destination strings */
  sub_len = e_idx - s_idx;
  dst_c = (char *) malloc(sub_len + 1);
  dst_asm = (char *) malloc(sub_len + 1);
  /* The following implies ENOMEM */
  if (!dst_c || !dst_asm) {
    perror("Failed to allocate destination buffers");
    return 4;
  }

  /* Call our sub_str routines (assigment is redundant but clear) */
  dst_c   =     sub_str(dst_c,   src, s_idx, e_idx);
  dst_asm = asm_sub_str(dst_asm, src, s_idx, e_idx);
  
  /* Ensure results are null terminated */
  dst_c[sub_len] = '\0';
  dst_asm[sub_len] = '\0';

  /* Output results, and indiciate if they match */
  printf("C   sub_str: '%s'\n"
         "ASM sub_str: '%s'\n"
         "Results %s\n",
         dst_c, dst_asm, (strcmp(dst_c, dst_asm) ? "Differ." : "Match!"));

  /* Free dynamically allocated memory (unnecessary but good practice) */
  free(dst_c);
  free(dst_asm);

  return 0;
}
