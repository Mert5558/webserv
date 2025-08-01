#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    int fd = open("index.html", O_RDONLY);  // Open the file
    if (fd == -1) {
        perror("error opening");
        return 1;
    }

    char buffer[6];  // buffer to read 5 characters + null terminator

    // First read: Read "hello"
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);  // Read up to 5 bytes
    if (bytes_read == -1) {
        perror("Error reading");
        close(fd);
        return 1;
    }

    buffer[bytes_read] = '\0';  // Null-terminate the string
    printf("First read (%zd bytes): %s\n", bytes_read, buffer);

    // Second read: Read " world" (5 characters after "hello")
    bytes_read = read(fd, buffer, sizeof(buffer) - 1);  // Read again
    if (bytes_read == -1) {
        perror("Error reading");
        close(fd);
        return 1;
    }

    buffer[bytes_read] = '\0';  // Null-terminate the string
    printf("Second read (%zd bytes): %s\n", bytes_read, buffer);

    // Third read: Reached EOF
    bytes_read = read(fd, buffer, sizeof(buffer) - 1);  // Try to read after EOF
    if (bytes_read == 0) {
        printf("EOF reached\n");  // Print EOF message
    } else if (bytes_read == -1) {
        perror("Error reading");
    }
	
    printf("Third read (%zd bytes): %s\n", bytes_read, buffer);

    close(fd);
    return 0;
}
