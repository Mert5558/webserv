#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 6  // Buffer size: 5 chars + 1 null terminator

int main() {
    int fd = open("index.html", O_RDONLY);  // Open the file
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    char buffer[BUFFER_SIZE];  // Create the buffer to read into
    ssize_t bytes_read = 0;
    size_t total_data = 0;  // Total data read in chunks

    // Loop to read the file in chunks
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) >= 0) {
        buffer[bytes_read] = '\0';  // Null-terminate the string after reading
        
        // Print the accumulated data from the buffer
        printf("Read chunk (%zd bytes): %s\n", bytes_read, buffer);
        if (bytes_read == 0) {
            perror("Error reading file");
            close(fd);
            return 1;
        }
        total_data += bytes_read;  // Accumulate the total bytes read
    }

    if (bytes_read == -1) {
        perror("Error reading file");
        close(fd);
        return 1;
    }

    printf("EOF reached. Total data read: %zd bytes\n", total_data);

    close(fd);
    return 0;
}
