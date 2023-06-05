#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void type_0(int num){
    int fd;
    char f_servo[4];
    char s_servo[4];

    switch (num)
    {
    case 0:
        strcpy(f_servo, "F180");
        strcpy(s_servo, "S180");
        break;

    case 1:
        strcpy(f_servo, "F180");
        strcpy(s_servo, "S180");
        break;

    case 2:
        strcpy(f_servo, "F180");
        strcpy(s_servo, "S180");
        break;

    case 3:
        strcpy(f_servo, "F180");
        strcpy(s_servo, "S180");
        break;

    case 4:
        strcpy(f_servo, "F180");
        strcpy(s_servo, "S180");
        break;

    case 5:
        strcpy(f_servo, "F180");
        strcpy(s_servo, "S180");
        break;

    case 6:
        strcpy(f_servo, "F180");
        strcpy(s_servo, "S180");
        break;

    case 7:
        strcpy(f_servo, "F180");
        strcpy(s_servo, "S180");
        break;

    case 8:
        fstrcpy(f_servo, "F180");
        strcpy(s_servo, "S180");
        break;

    case 9:
        strcpy(f_servo, "F180");
        strcpy(s_servo, "S180");
        break;
    
    default:
        break;
    }


    ssize_t s_written;
    ssize_t f_written;

    fd = open("/dev/ttyUSB0", O_RDWR);

    if (fd == -1){
        perror("Error al abrir el dispositivo");
        return 1;
    }

    f_written = write(fd, f_servo, sizeof(f_servo) - 1);
    
    if (f_written == -1) {
        perror("Error al escribir en el dispositivo");
        close(fd);
        return 1;
    }

    s_written = write(fd, s_servo, sizeof(s_servo) - 1);
    
    if (s_written == -1) {
        perror("Error al escribir en el dispositivo");
        close(fd);
        return 1;
    }
    close(fd);
    
    default_pos();
}


void default_pos(){
    int fd;
    char f_servo[] = "F180";
    char s_servo[] = "S180";

    ssize_t s_written;
    ssize_t f_written;

    fd = open("/dev/ttyUSB0", O_RDWR);

    if (fd == -1){
        perror("Error al abrir el dispositivo");
        return 1;
    }

    f_written = write(fd, f_servo, sizeof(f_servo) - 1);
    
    if (f_written == -1) {
        perror("Error al escribir en el dispositivo");
        close(fd);
        return 1;
    }

    s_written = write(fd, s_servo, sizeof(s_servo) - 1);
    
    if (s_written == -1) {
        perror("Error al escribir en el dispositivo");
        close(fd);
        return 1;
    }

    close(fd);
}