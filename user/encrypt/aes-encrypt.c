#include "aes-encrypt.h"
unsigned char key[] = "0123456789abcdef";
unsigned char iv[] = "fedcba9876543210";

unsigned char* encrypt(int inputChar) {

    // create an input buffer for the AES algorithm
    unsigned char inputBuf[AES_BLOCK_SIZE];
    memset(inputBuf, 0, AES_BLOCK_SIZE);
    inputBuf[0] = (unsigned char)inputChar;

    // set up the encryption context
    
    AES_KEY aes_key;
    AES_set_encrypt_key(key, 128, &aes_key);

    // perform AES encryption on the input data
    unsigned char* output = malloc(AES_BLOCK_SIZE * sizeof(char));
    AES_cbc_encrypt(inputBuf, output, AES_BLOCK_SIZE, &aes_key, iv, AES_ENCRYPT);

    return output;
}
