#ifndef AESENCRYPT_H
#define AESENCRYPT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/aes.h>
#include <ctype.h>

<<<<<<< Updated upstream
unsigned char* encrypt(int inputChar);

=======
unsigned char* encrypt(const char* inputString);
unsigned char* decrypt(unsigned char* encryptedData);
>>>>>>> Stashed changes

#endif