#include <stdio.h>

extern int add2(int i, int j);
 
int main(int argc, char **argv) {
    int i = 5, j = 10, result;
     
    result = add2(i, j);
    printf("result: %d\n", result);

    return 0;
}
