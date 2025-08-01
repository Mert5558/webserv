#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() {
	int fd = open ("index.html", O_RDONLY);

	if (fd == -1) {
		perror("error opening");
		return 1;
	}

	char buffer[5];
	ssize_t bytes_read = read(fd, buffer, sizeof(buffer)); // This reads only 40 bytes, not related to buffer size
	printf("bytes read %lu\n", bytes_read);
	if (bytes_read == -1) {
		perror("we got problem");
		close(fd);
		return -1;
	}

	buffer[bytes_read] = '\0';
	printf("read %zd bytes:%s \n", bytes_read, buffer);
	close(fd);
	return 0;
}