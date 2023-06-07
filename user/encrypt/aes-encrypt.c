#include "aes-encrypt.h"
unsigned char key[] = "0123456789abcdef";
unsigned char iv[] = "fedcba9876543210";

unsigned char* encrypt(const char* inputString) {

    // create an input buffer for the AES algorithm
    unsigned char inputBuf[AES_BLOCK_SIZE];
    memset(inputBuf, 0, AES_BLOCK_SIZE);

     // Copy the inputString into the input buffer
    strncpy((char*)inputBuf, inputString, AES_BLOCK_SIZE);
    
    AES_KEY aes_key;
    AES_set_encrypt_key(key, 128, &aes_key);

    // perform AES encryption on the input data
    unsigned char* output = malloc(AES_BLOCK_SIZE * sizeof(unsigned char));
    AES_cbc_encrypt(inputBuf, output, AES_BLOCK_SIZE, &aes_key, iv, AES_ENCRYPT);

    return output;
}
<<<<<<< Updated upstream
=======
unsigned char* decrypt(unsigned char* encryptedData) {
    unsigned char outputBuf[AES_BLOCK_SIZE];
    memset(outputBuf, 0, AES_BLOCK_SIZE);

    AES_KEY aes_key;
    AES_set_decrypt_key(key, 128, &aes_key);

    AES_cbc_encrypt(encryptedData, outputBuf, AES_BLOCK_SIZE, &aes_key, iv, AES_DECRYPT);

    unsigned char* decryptedData = malloc(AES_BLOCK_SIZE * sizeof(unsigned char));
    if (decryptedData == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    memcpy(decryptedData, outputBuf, AES_BLOCK_SIZE);

    return decryptedData;
}
>>>>>>> Stashed changes
