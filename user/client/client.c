#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"       //local host
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

    char buf[3];
    while (1) {
        printf("Enter button number to press: ");
        fgets(buf, sizeof(buf), stdin);

        // send message to server
        if (sendto(client_socket, buf, strlen(buf), MSG_CONFIRM,
                    (const struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            printf("Error sending the number to server. sendto() error");
            exit(EXIT_FAILURE);
        }

        printf("Message sent...\n");
        // clear the buffer
        memset(buf, 0, sizeof(buf));
    }

    close(client_socket);
    return 0;
}
