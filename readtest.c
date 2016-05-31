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

	printf("Starting read test...\n");
	fd = open("/dev/charfs", O_RDONLY);

	if (fd < 0) {
		perror("Failed to open the device...");
		return errno;
	}

	printf("Reading from the device...\n");

	result = read(fd, buffer, BUFFER_LENGTH);        // Read the response from the LKM

	printf("Result: %zd\n", result);

	if (result < 0) {
		perror("Failed to read the message from the device.");
		return errno;
	}

	if (strlen(buffer) < 50) {
		printf("The received message is: '%s'\n", buffer);
	}
	else {
		buffer[50] = 0;
		printf("The received message is: '%s' (truncated)\n", buffer);
	}

//	free(buffer); // Freeing here gives a 'double free or corruption' error

	return 0;
}
