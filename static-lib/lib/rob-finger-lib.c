#include "rob-finger-lib.h"

int openFingDriver(const char* device_path){
    int fd = open(device_path, O_RDWR);
    
    if (fd == -1) {
        perror("Failed to open the device");
    }
    
    return fd;
}
void moveS1(int fd, int deg){
    char command[5];
    int ret;

    ret = snprintf(command, sizeof(command), "F%d-", deg);
    if (ret < 0 || ret >= sizeof(command)) {
        fprintf(stderr, "Failed to create command\n");
        return;
    }

    int num_bytes = write(fd, command, strlen(command));
    if (num_bytes == -1) {
        perror("Failed to write to the device");
        return;
    }
    // Check if all bytes were written
    if (num_bytes < strlen(command)) {
        fprintf(stderr, "Failed to write all data to the device\n");
        return;
    }
}
void moveS2(int fd, int deg){
    char command[5];
    int ret;

    ret = snprintf(command, sizeof(command), "S%d-", deg);
    if (ret < 0 || ret >= sizeof(command)) {
        fprintf(stderr, "Failed to create command\n");
        return;
    }

    int num_bytes = write(fd, command, strlen(command));
    if (num_bytes == -1) {
        perror("Failed to write to the device");
        return;
    }
    // Check if all bytes were written
    if (num_bytes < strlen(command)) {
        fprintf(stderr, "Failed to write all data to the device\n");
        return;
    }
}
void pressEnter(int fd) {

}
void closeFingDriver(int fd){
    if (close(fd) == -1) {
        perror("Failed to close the device");
        // You can add further error handling here if needed
    }
}