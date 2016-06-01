#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LENGTH 4096

int main(void) {
	int fd;
	ssize_t result;

	char *buffer = malloc(BUFFER_LENGTH); // If this is not on the heap we get a '-14 Bad address' error

	printf("Starting write test...\n");
	fd = open("/dev/charfs", O_WRONLY);

	if (fd < 0) {
		perror("Failed to open the device...");
		return errno;
	}

	sprintf(buffer, "Hello! This is a string written from user space.");

	printf("Writing to the device...\n");

	result = write(fd, buffer, BUFFER_LENGTH);        // Read the response from the LKM

	printf("Result: %zd\n", result);

	if (result < 0) {
		perror("Failed to write the message to the device.");
		return errno;
	}

	free(buffer);

	return 0;
}
