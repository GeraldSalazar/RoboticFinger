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
void pressButtonE(int numero);
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
    // Inicializar el semáforo
    // sem_t *sem;
    
    // // Crear sem con nombre "sem" y un valor inicial de 1
    // sem = sem_open("/sem", O_CREAT | O_EXCL, 0644, 1);
    // if (sem == SEM_FAILED) {
    //    perror("sem_open");
    //    exit(1);
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
                pressButtonE(10);
                break;
            case '1':
                pressButtonE(20);
                break;
            case '2':
                pressButtonE(30);
                break;
            case '3':
                pressButtonE(40);
                break;
            case '4':
                pressButtonE(50);
                break;
            case '5':
                pressButtonE(60);
                break;
            case '6':
                pressButtonE(70);
                break;
            case '7':
                pressButtonE(80);
                break;
            case '8':
                pressButtonE(90);
                break;
            case '9':
                pressButtonE(100);
                break;
            default:
                printf("Carácter no reconocido\n");
                break;
        }
    }
    
}
void pressButtonE(int numero){
    int fd = openFingDriver(device_path);
    moveS1(fd, numero); 
    closeFingDriver(fd); //Servo azul, izq - der
    sleep(2);
    
    fd = openFingDriver(device_path);
    moveS2(fd, 100); //Servo negro pos neutral
    moveS2(fd, 60);  //Bajar
    closeFingDriver(fd);
    sleep(2);
    
    fd = openFingDriver(device_path);
    moveS2(fd, 100);  //Subir
    closeFingDriver(fd);
    sleep(2);
}