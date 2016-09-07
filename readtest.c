#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
//#include <malloc.h>

#define BUFFER_LENGTH 8192 // Should be a multiple of NVMe block size, as we DMA straight to it without checking

char buffer[BUFFER_LENGTH] __attribute__((aligned(0x1000)));

int main(void) {
	int fd, outfd;
	ssize_t result;

//	char *buffer = malloc(BUFFER_LENGTH);
//	char *buffer = memalign(4096, BUFFER_LENGTH);

	printf("Starting read test...\n");
	fd = open("/dev/charfs", O_RDONLY);

	if (fd < 0) {
		perror("Failed to open the device");
		return errno;
	}

	printf("Reading from the device...\n");

	result = read(fd, buffer, BUFFER_LENGTH);

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

	outfd = open("readtest-out", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
	if (outfd < 0) {
		perror("Failed to open output file");
		return errno;
	}
	write(outfd, buffer, BUFFER_LENGTH);
	close(outfd);

//	free(buffer); // Freeing here gives a 'double free or corruption' error?

	return 0;
}
