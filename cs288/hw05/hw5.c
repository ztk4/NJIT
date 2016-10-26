#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_LENGTH 100

struct clip *build_a_lst();
struct clip *append();
int find_length();
void print_lst();
void split_line();
void free_lst();

struct clip {
  int views;
  char *user;
  char *time;
  char *id;
  char *title;
  struct clip *next;
} *head;

int main(int argc, char **argv) {
  int n;
  head = build_a_lst(*(argv+1));
  n = find_length(head);
  printf("%d clips\n",n);
  print_lst(head);		/* prints the table */

  free_lst(head);
  return 0;
}

struct clip *build_a_lst(char *fn) {
  FILE *fp;
  struct clip *hp;
  char *fields[5];
  char line[LINE_LENGTH];
  hp=NULL;

  fp = fopen(fn, "r");
  while (fgets(line, LINE_LENGTH, fp)) {
    split_line(fields, line);
    hp = append(hp, fields);
    
    // Ignore rest of line
    char c;
    while ( (c = fgetc(fp)) != '\n' && c != EOF);
  }

  return hp;
}

/* fields will have five values stored upon return */
void split_line(char **fields,char *line) {
  char *token, *delim;
  delim = ",\n";
  
  token = strtok(line, delim);
  do {
    *fields++ = token;
  } while (token = strtok(NULL, delim));
}

/* set four values into a clip, insert a clip at the of the list */
struct clip *append(struct clip *hp,char **fields) {
  struct clip *cp,*tp;

  tp = (struct clip *) malloc(sizeof(struct clip));
  tp->views = atoi(fields[1]);

  tp->user = (char *) malloc(sizeof(char) * (strlen(fields[0]) + 1));
  tp->time = (char *) malloc(sizeof(char) * (strlen(fields[2]) + 1));
  tp->id = (char *) malloc(sizeof(char) * (strlen(fields[3]) + 1));
  tp->title = (char *) malloc(sizeof(char) * (strlen(fields[4]) + 1));

  tp->next = NULL;

  strcpy(tp->user, fields[0]);
  strcpy(tp->time, fields[2]);
  strcpy(tp->id, fields[3]);
  strcpy(tp->title, fields[4]);

  // Append to empty list
  if (!hp) {
    return tp;
  }

  cp = hp;
  while (cp->next) cp = cp->next;
  cp->next = tp;

  return hp;
}

void print_lst(struct clip *cp) {
  while (cp) {
     printf("%d,%s,%s,%s,%s\n",cp->views,cp->user,cp->id,cp->title,cp->time);
     cp = cp->next;
  }
}

int find_length(struct clip *cp) {
  int len = 0;
  while (cp) {
    ++len;
    cp = cp->next;
  }

  return len;
}

void free_lst(struct clip *cp) {
  struct clip *tmp;
  while (cp) {
    tmp = cp->next;

    free(cp->user);
    free(cp->time);
    free(cp->id);
    free(cp->title);
    free(cp);

    cp = tmp;
  }
}

/* end */
