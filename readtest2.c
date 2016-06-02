#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define READ_LENGTH 8192
#define BUFFER_LENGTH READ_LENGTH + 4096

char buffer[BUFFER_LENGTH]; // Allocate a buffer that is 4k bigger than the read size

int main(void) {
	int fd, outfd;
	ssize_t result;

	printf("Starting read test...\n");

	memset(buffer, 0xff, BUFFER_LENGTH);

	outfd = open("before-low", O_WRONLY | O_CREAT | O_TRUNC);
	if (outfd < 0) {
		perror("Failed to open output file");
		return errno;
	}
	write(outfd, buffer, READ_LENGTH);
	close(outfd);

	outfd = open("before-high", O_WRONLY | O_CREAT | O_TRUNC);
	if (outfd < 0) {
		perror("Failed to open output file");
		return errno;
	}
	write(outfd, buffer + READ_LENGTH, BUFFER_LENGTH - READ_LENGTH);
	close(outfd);

	fd = open("/dev/charfs", O_RDONLY);

	if (fd < 0) {
		perror("Failed to open the device");
		return errno;
	}

	printf("Reading from the device...\n");

	result = read(fd, buffer, READ_LENGTH);

	printf("Result: %zd\n", result);

	if (result < 0) {
		perror("Failed to read the message from the device");
		return errno;
	}

	if (strlen(buffer) < 50) {
		printf("The received message is: '%s'\n", buffer);
	}
	else {
		buffer[50] = 0;
		printf("The received message is: '%s' (truncated)\n", buffer);
	}

	outfd = open("after-low", O_WRONLY | O_CREAT | O_TRUNC);
	if (outfd < 0) {
		perror("Failed to open output file");
		return errno;
	}
	write(outfd, buffer, READ_LENGTH);
	close(outfd);

	outfd = open("after-high", O_WRONLY | O_CREAT | O_TRUNC);
	if (outfd < 0) {
		perror("Failed to open output file");
		return errno;
	}
	write(outfd, buffer + READ_LENGTH, BUFFER_LENGTH - READ_LENGTH);
	close(outfd);

	return 0;
}
