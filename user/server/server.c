#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <sys/wait.h>
#include "aes-encrypt.h"
#include "/home/daval/Escritorio/Sistemas_operativos/Proyecto_3/RoboticFinger/static-lib/include/rob-finger-lib.h"

#define PORT 8080
const char* device_path = "/dev/ttyUSB0";

void mapNum(unsigned char *numero);

int main() {
    struct sockaddr_in servaddr, cliaddr;
    int number;
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
    //El archivo del driver
    int fd = openFingDriver(device_path);
    
    // Inicializar el semáforo
    //sem_t *sem;
    
    // Crear sem con nombre "sem" y un valor inicial de 1
    //sem = sem_open("/sem", O_CREAT | O_EXCL, 0644, 1);
    //if (sem == SEM_FAILED) {
    //    perror("sem_open");
    //    exit(1);
    //}

    // if (fd != -1) {
    //     moveS1(fd, 90);
    //     //sleep(1000);
    //     //moveS1(fd, 0);
    //     //sleep(1000);
    //     //moveS2(fd, 1);
    //     //pressEnter(fd);
    //     closeFingDriver(fd);
    // }
    unsigned char* decrypted_data;

    printf("UDP server listening on port %d\n", PORT);
    while (1) {
        unsigned char received_data[AES_BLOCK_SIZE];
        memset(received_data, 0, sizeof(received_data));
        unsigned int len = sizeof(cliaddr);
        
        // Receive message from client
        int n = recvfrom(sockfd, received_data, sizeof(received_data), MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
        if (n < 0 ) {
            printf("Couldn't receive message from client. recvfrom() failed");
            exit(EXIT_FAILURE);
        }
        received_data[n] = '\0';
        for (int i = 0; i < sizeof(received_data); i++) {
            printf("%02x", received_data[i]);
        }
        printf("\n");
        
        // Decrypt the received data
        decrypted_data = decrypt(received_data);
        decrypted_data[AES_BLOCK_SIZE]='\0';
        printf("Decrypted num pad buttons: %s\n", decrypted_data);
        
        ///////////////////////////////
        //sem_wait(sem);
        
        pid_t pid = fork();
        if(pid == 0){
            mapNum(decrypted_data);
            exit(0);
        }else{
            wait(NULL);
        }
        
        //sem_post(sem);
        ///////////////////////////////

        // Free the memory allocated for decrypted data
        free(decrypted_data);    
        
    }

    close(sockfd);
    return 0;
}
void mapNum(unsigned char *numero){
    for (int i = 0; numero[i] != '\0'; i++) {
        switch (numero[i]) {
            case '0':
                printf("Cero\n");
                break;
            case '1':
                printf("Uno\n");
                break;
            case '2':
                printf("Dos\n");
                break;
            case '3':
                printf("Tres\n");
                break;
            case '4':
                printf("Cuatro\n");
                break;
            case '5':
                printf("Cinco\n");
                break;
            case '6':
                printf("Seis\n");
                break;
            case '7':
                printf("Siete\n");
                break;
            case '8':
                printf("Ocho\n");
                break;
            case '9':
                printf("Nueve\n");
                break;
            default:
                printf("Carácter no reconocido\n");
                break;
        }
    }
    
}