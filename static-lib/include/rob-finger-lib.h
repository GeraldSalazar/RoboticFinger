
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int openFingDriver(const char* device_path);
void moveS1(int fd, int deg);
void moveS2(int fd, int deg);
void pressEnter(int fd);
void closeFingDriver(int fd);
