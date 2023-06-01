#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 256

int main()
{
    int fd;
    char write_data[] = "180";
    char read_data[BUFFER_SIZE];
    ssize_t num_written, num_read;

    // Abrir el dispositivo FT232R USB
    fd = open("/dev/ttyUSB0", O_RDWR);
    if (fd == -1) {
        perror("Error al abrir el dispositivo");
        return 1;
    }

    // Escribir datos en el dispositivo
    num_written = write(fd, write_data, sizeof(write_data) - 1);
    if (num_written == -1) {
        perror("Error al escribir en el dispositivo");
        close(fd);
        return 1;
    }
    // Cerrar el dispositivo
    close(fd);

    return 0;
}
