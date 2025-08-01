#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    int fd = open("index.html", O_RDONLY); // Open file for reading
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    char buffer[100];  // Buffer to store the read data
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
    if (bytesRead == -1) {
        perror("Error reading file");
        close(fd);
        return 1;
    }

    printf("Read %zd bytes: %.*s\n", bytesRead, (int)bytesRead, buffer);
    close(fd);  // Close the file after reading
    return 0;
}
