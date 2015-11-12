#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <openssl/md5.h>

int main(int argc, char **argv) {
	if(argc != 2) {
		printf("Usage: %s passwd\n", argv[0]);
		return 1;
	}

	char passwd[21]; 
	strncpy(passwd, argv[1], 20); //cpy to buffer to ensure password is resonably sized
	passwd[20] = '\0'; //add null byte at end in case password was >= 20 in length

	uint8_t hash[16];

	MD5(passwd, strlen(passwd), (char *)hash);
	
	printf("Your password '%s' hashes to: uint8_t hash[] = {", passwd);
	for(int i = 0; i < 16; ++i) {
		printf("%d, ", hash[i]);
	}
	printf("\b\b};\n");

	return 0;
}
