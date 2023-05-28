#ifndef AESENCRYPT_H
#define AESENCRYPT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/aes.h>
#include <ctype.h>

unsigned char* encrypt(int inputChar);
unsigned char decrypt(unsigned char* encryptedData);

#endif