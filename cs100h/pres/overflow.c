#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <openssl/md5.h>

void grant_shell() {
	printf("Welcome back superuser!\nHere's your shell:\n");
	setuid(geteuid());
	char *const args[] = {"/bin/sh",  NULL};
	execv(args[0], args);
}

uint8_t valid_hash[] = {193, 179, 147, 105, 178, 122, 16, 111, 31, 197, 119, 133, 91, 193, 217, 252}; //easy way of storing hash in source file (16 bytes represented as two unsigned 8 byte integers)

int verify(char *argv1) {
	char passwd[21]; 			//holds 20 characters plus a null byte
	memset(passwd, '\0', 20); 	//set the whole string to null bytes (except the last char)
	strcpy(passwd, argv1);
	passwd[20] = '\0'; 			//set the last char to null, ensures the string ends!

	uint8_t hash[16];			//enough space for 16 byte hash
	MD5(passwd, strlen(passwd), (char *)hash);

	return !memcmp(hash, valid_hash, 16);
}

int main(int argc, char **argv) {
	if(argc != 2) {
		printf("Usage: %s passwd\n", argv[0]);
	}

	if(verify(argv[1])) { //if password is okay
		grant_shell();
	} else {
		printf("You're not the superuser, you don't know the password!!\n");
	}
	
	return 0;
}
