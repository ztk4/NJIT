#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

// Global Constants
#define CELL_WIDTH 6
#define NUM_CELLS 8
#define LABEL_WIDTH 8
#define LINE_WIDTH (LABEL_WIDTH + CELL_WIDTH * NUM_CELLS + NUM_CELLS + 1)

// Mask "Function" Macros
#define MASK_ALL 0xFF
#define MASK_LEFTMOST(n) ((char) (0xFF << (8 - (n))))
#define MASK_RIGHTMOST(n) (0xFF >> (8 - (n)))

// Creates a line between memory cells.
void put_line() {
  char line[LINE_WIDTH + 1];

  memset(line, ' ', LABEL_WIDTH);

  char *pos = line + LABEL_WIDTH;

  for (int i = 0; i < NUM_CELLS; ++i) {
    *pos++ = '+';
    memset(pos, '-', CELL_WIDTH);
    pos += CELL_WIDTH;
  }

  *pos++ = '+';
  *pos = '\0';

  printf("%s", line);
}

// Centers in, in out, by spaces
// out is at least size + 1 chars long (for null byte)
void center_str(char *out, char *in, int size) {
  int len = strlen(in);
  int diff = size - len;
  if (diff < 0) return;  // output too small

  int padding = diff / 2;

  // Prefixed Pading
  memset(out, ' ', padding);

  // Contents
  strcpy(out + padding, in);

  // Suffixed Padding (plus 1 if diff is odd)
  memset(out + padding + len, ' ', padding + (diff & 1));

  // Set Null Byte
  out[size] = '\0';
}

// Creates a row of cells.
// label may be NULL to specify no label.
// address The range [address, address + NUM_CELLS) will be printed.
// to_str is a function handler that converts chars to strings of length CELL_WIDTH
//        the first argument is the address to store the output in
// mask The bits that are set in mask are read, the rest are filled in with bogus
void put_cells(char *label, char *address, void (*to_str)(char *, char), char mask) {
  if (!label) 
    label = "";
  printf("%-*s", LABEL_WIDTH, label);
  
  printf("|");
  char bogus[CELL_WIDTH + 1];
  char buf[CELL_WIDTH + 1];

  // Generate Bogus Cells
  if (mask != 0xFF) {
    center_str(bogus, "--", CELL_WIDTH);
  }
      
  // Actual Cells
  for (char *c = address + NUM_CELLS - 1; c >= address; --c, mask <<= 1) {
    if (mask & 0x80) {  // Checks the most significant bit
      to_str(buf, *c);
      printf("%s|", buf);
    } else {
      printf("%s|", bogus);
    }
  }

  printf(" %016" PRIXPTR "\n", (uintptr_t) address);
}

void to_hex(char *out, char c) {
  char hex[3];
  snprintf(hex, 3, "%02X", (unsigned char) c);

  center_str(out, hex, CELL_WIDTH);
}

void to_txt(char *out, char c) {
  char txt[3];
  if (c) {
    snprintf(txt, 3, "%c", c);
  } else {
    strcpy(txt, "\\0");
  }

  center_str(out, txt, CELL_WIDTH);
}

// Prints the stack with argc and argv stuff expanded
// to_str will be used to print characters from memory
void print_stack(int *argc_p, char ***argv_p, void (*to_str)(char *, char)) {
  put_line();
  printf(" %016" PRIXPTR "\n", (uintptr_t) (NULL - 1));  // NULL - 1 overflows to "max" address
  for (int i = 0; i < 3; ++i) {
    put_line();
    printf("\n");
  }

  // Get a pointer to the null byte of the last char * in argv
  char *last = strchr((*argv_p)[*argc_p - 1], '\0');
  unsigned int offset = (uintptr_t) last & 7;  // Offset of last from word allignment
  char *address = last - offset;
  // Print the rightmost offset + 1 (including nullbyte) cells
  put_cells(NULL, address, to_str, MASK_RIGHTMOST(offset + 1));

  for (address -= 8; address >= **argv_p; address -= 8) {
    put_line();
    printf("\n");

    put_cells(NULL, address, to_str, MASK_ALL);
  }

  // Offset of begining of first element of argv from word alligment
  offset = (uintptr_t) **argv_p & 7;
  // If offset is 0, then the block has already been printed above
  if (offset) {
    put_line();
    printf("\n");

    // Print everything but the offset
    put_cells(NULL, **argv_p - offset, to_str, MASK_LEFTMOST(8 - offset));
  }

  for (int i = 0; i < 4; ++i) {
    put_line();
    printf("\n");
  }

  put_cells(NULL, (char *) (*argv_p + *argc_p), &to_hex, MASK_ALL);  // NULL after argv
  for (int i = *argc_p - 1; i >= 0; --i) {
    put_line();
    printf("\n");
    
    char label[LABEL_WIDTH + 1];
    snprintf(label, LABEL_WIDTH + 1, "argv+%d", i);
    put_cells(label, (char *) (*argv_p + i), &to_hex, MASK_ALL);
  }
  
  for (int i = 0; i < 4; ++i) {
    put_line();
    printf("\n");
  }

  // argc is in upper have of word, so mask is leftmost 4 bytes,
  // and address must be shifted down 4 bytes to be a multiple of 8
  put_cells("argc", (char *) argc_p - 4, &to_hex, MASK_LEFTMOST(4));
  put_line();
  printf("\n");
  put_cells("argv", (char *) argv_p, &to_hex, MASK_ALL);

  for (int i = 0; i < 3; ++i) {
    put_line();
    printf("\n");
  }

  put_line();
  printf(" %016" PRIXPTR "\n", (uintptr_t) NULL);
}

int main(int argc, char **argv) {
  printf("Memory map filled with hexadecimals only:\n");
  print_stack(&argc, &argv, &to_hex);

  printf("\n\n");

  printf("Memory map filled with the command and parameters in ASCII text and others in hexadecimals:\n");
  print_stack(&argc, &argv, &to_txt);

  return 0;
}
