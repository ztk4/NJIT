/* decrypt the string encrypted via Caesar shift */
#include <stdio.h>
#include <ctype.h> /*isalpha for determining when to shift*/
#include <stdlib.h> /*free & malloc for pointers*/

extern void init(char *encrypt);
extern char *decrypt(char *encrypt, int shift);

int main(int argc, char *argv[]) {
    char *result;          /* pointer to decrypted string */
    char encrypt[] = "GSRKVEXYPEXMSRW CSY LEZI JSYRH XLI WLMJX ZEPYI" ;
    int i;

    init(encrypt);
    for(i = 1; i < 5; i++) {     /* loop to try various shifts */
        result = decrypt(encrypt, i);
        printf("Possible decrypt with shift %d: %s\n", i, result);
    }
    free(result); /*free the pointer allocated in init (retured to result each time in loop)*/

    return 0;
}

