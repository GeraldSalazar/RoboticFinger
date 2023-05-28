#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "aes-encrypt.h"

#define PORT 8080

int main() {
    struct sockaddr_in servaddr, cliaddr;

    // Socket file descriptor. SOCK_DGRAM for the User Datagram Protocol (UDP). 
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Server information
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;          // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;  // Any available interface
    servaddr.sin_port = htons(PORT);


    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {
        printf("Couldn't bind the socket");
        exit(EXIT_FAILURE);
    }

    printf("UDP server listening on port %d\n", PORT);

    while (1) {

        unsigned char received_data[AES_BLOCK_SIZE];
        unsigned int len = sizeof(cliaddr);
        // Receive message from client
        int n = recvfrom(sockfd, received_data, sizeof(received_data), MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
        if (n < 0 ) {
            printf("Couldn't receive message from client. recvfrom() failed");
            exit(EXIT_FAILURE);
        }
        received_data[n] = '\0';
        printf("\033[32m Encrypted data received from client: \033[0m ");
        for (int i = 0; i < sizeof(received_data); i++) {
            printf("%02x", received_data[i]);
        }
        printf("\n");
        //desencrypt
        unsigned char decryptedChar = decrypt(received_data);
        printf("Decrypted num pad button: %c\n", (char)decryptedChar);

        //pasar num a biblioteca
    }

    close(sockfd);
    return 0;
}

// Send response back to client (just in case)
//char *message = "Hello from server!";
//sendto(sockfd, (const char *)message, strlen(message), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
//printf("Response sent to %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));