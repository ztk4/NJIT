#include <stdio.h>
#include <stdlib.h>

extern int replace(char *s, char lr, char rp);

int main(int argc, char **argv) {
    int result;
    char lr = 'e';
    char rp = 'X';
    char str[] = "easy to replace";
    
    result = replace(str, lr, rp);
    printf("%d changes, %s\n", result, str);
    
    exit(0);
}
