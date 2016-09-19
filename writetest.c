#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
//#include <malloc.h>

#define BUFFER_LENGTH 4096 // Should be a multiple of NVMe block size, as we DMA straight from it without checking

char buffer[BUFFER_LENGTH];

int main(void) {
	int fd;
	ssize_t result;

//	char *buffer = malloc(BUFFER_LENGTH);
//	char *buffer = memalign(4096, BUFFER_LENGTH);

	printf("Starting write test...\n");
	fd = open("/dev/chardisk0", O_WRONLY);

	if (fd < 0) {
		perror("Failed to open the device");
		return errno;
	}

	sprintf(buffer, "Hello! This is a string written from user space.");

	printf("Writing to the device...\n");

	result = write(fd, buffer, BUFFER_LENGTH);

	printf("Result: %zd\n", result);

	if (result < 0) {
		perror("Failed to write the message to the device");
		return errno;
	}

//	free(buffer);

	return 0;
}
