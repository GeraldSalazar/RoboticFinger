#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 2

// El proceso para presionar el 1.
// WriteServo1(18);
// sleep(2s);
// WriteServo2(90);
// sleep(1s);
// WriteServo2(0);
// sleep(1s);
// WriteServo1(0);

int main()
{
    int fd;
    char write_data[] = "S100-";   //F- First servo (Azul), S-Second servo (negro)
    char read_data[BUFFER_SIZE];
    ssize_t num_written, num_read;

    //ttyUSB0
    //fingDriver
    // Abrir el dispositivo FT232R USB
    fd = open("/dev/ttyUSB0", O_RDWR);
    if (fd == -1) {
        perror("Error al abrir el dispositivo");
        close(fd);
        return 1;
    }
    // Escribir datos en el dispositivo
    num_written = write(fd, write_data, sizeof(write_data) - 1);
    if (num_written == -1) {
        perror("Error al escribir en el dispositivo");
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}
