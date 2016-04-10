#include <stdlib.h>
#include <stdio.h>

extern char *int2hex(int convert);

int main(int argc, char **argv) {
    int convert = 1194684;
    char *result;
    
    result = int2hex(convert);
    printf("Integer to hex string: %s\n", result);

    free(result);

    return 0;
}
