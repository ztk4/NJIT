#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void split_line(char **fields, char *line) {
  char *token, *delim;
  delim = ",";

  token = strtok(line, delim);
  while (token) {
    *fields = (char *) malloc(strlen(token) * sizeof(char));
    strcpy(*fields++, token);
    token = strtok(NULL, delim);
  }
}

int main(int argc, char **argv) {
  char *fields[5];
  char str[] = "This,is a,test\nof,my,code!";
  printf("Original string is '%s'\n", str);

  split_line(fields, str);

  for (char **field = fields; field < fields + 5; ++field) {
    printf("Got token '%s'\n", *field);
    free(*field);
  }

  return 0;
}
