#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <semaphore.h>

#include "aes-encrypt.h"

#define SERVER_IP "127.0.0.1"       //localhost
#define SERVER_PORT 8080
#define MAX_INPUT_LENGTH 10        // Maximum length of input string

int main() {
    struct sockaddr_in server_address;

    // create UDP socket
    int client_socket;
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // fill in server address information
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
        perror("Invalid address or not supported");
        exit(EXIT_FAILURE);
    }
     /* Inicializar semáforos */
    sem_t *sem;
    sem = sem_open("/sem",0);
    while(1) {

        char inputString[MAX_INPUT_LENGTH];

        // input validation loop
        int invalid = 0;
        while (1) {
            printf("Enter the numpad button(s) to press: ");
            fflush(stdin); // clear the input buffer
            fgets(inputString, sizeof(inputString), stdin);

            // Remove trailing newline character
            inputString[strcspn(inputString, "\n")] = '\0';

            // Validate each character in the input string
            for (int i = 0; inputString[i] != '\0'; i++) {
                char currentChar = inputString[i];
                if (!(isdigit(currentChar) || currentChar == 'E' || currentChar == 'e')) {
                    invalid = 1;
                    break;
                }
            }

            if (!invalid) {
                break;
            }

            invalid = 0;
            printf("\033[31mInvalid input. Please type E (enter) or a number between 0 and 9.\033[0m\n");
        }

        // Encrypt the input data
        unsigned char* encrypted_data = encrypt(inputString);
        // print the encrypted output
        printf("Encrypted output: ");
        for (int i = 0; i < AES_BLOCK_SIZE; i++) {
            printf("%02x", encrypted_data[i]);
        }
        printf("\n");
        /////////////////////////////////////////////////////////
        if (sendto(client_socket, encrypted_data, strlen(encrypted_data) + 1, MSG_CONFIRM,
                    (const struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            printf("Error sending the number to server. sendto() error");
            exit(EXIT_FAILURE);
        }

        printf("\033[32m Message sent...\033[0m\n");
        /////////////////////////////////////////////////////////

        // Free the memory allocated for the encrypted data
        free(encrypted_data);

    }

    close(client_socket);
    return 0;
}