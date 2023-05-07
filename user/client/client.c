#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "aes-encrypt.h"

#define SERVER_IP "127.0.0.1"       //localhost
#define SERVER_PORT 8080

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

    while(1) {

        // input validation loop 
        int inputChar;
        int nextChar = '\0';
        int invalid = 0;
        while(1){   // keep asking for the button to be pressed
            printf("Enter the numpad button to press: ");
            fflush(stdin); // clear the input buffer
            inputChar = getchar();
            while ((nextChar = getchar()) != '\n' && nextChar != EOF) { // Discard remaining characters and validate cases like 'ee', 44, 11, 'EE', etc
                if(isdigit(nextChar) || (nextChar == 'E') || (nextChar == 'e')){
                    invalid = 1;
                }
            } 
            // validate the user input. Should be a numeric digit or the char "E"
            if (((inputChar >= '0' && inputChar <= '9') || (inputChar == 'E') || (inputChar == 'e')) && !invalid) {
                break;
            }
            invalid = 0;
            printf("\033[31m Invalid input. Please type E (enter) or a number between 0 and 9.\033[0m\n");
        }

        // Encrypt the input data
        unsigned char* encrypted_data = encrypt(inputChar);
        // print the encrypted output
        printf("Encrypted output: ");
        for (int i = 0; i < AES_BLOCK_SIZE; i++) {
            printf("%02x", encrypted_data[i]);
        }
        printf("\n");

        // send message to server
        if (sendto(client_socket, encrypted_data, strlen(encrypted_data), MSG_CONFIRM,
                    (const struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            printf("Error sending the number to server. sendto() error");
            exit(EXIT_FAILURE);
        }

        printf("\033[32m Message sent...\033[0m\n");
        // clear the buffer

        // Free the memory allocated for the encrypted data
        free(encrypted_data);

    }

    close(client_socket);
    return 0;
}
