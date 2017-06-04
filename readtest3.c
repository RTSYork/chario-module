#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
//#include <malloc.h>

#define BUFFER_LENGTH (4096*1024*10) // Should be a multiple of NVMe block size, as we DMA straight to it without checking

char buffer[BUFFER_LENGTH] __attribute__((aligned(0x1000)));

int main(int argc, char **argv) {
	int fd, outfd;
	size_t size;
	ssize_t result;

	if (argc < 2) {
		size = 4096;
	}
	else {
		size = (size_t)strtol(argv[1], (char **)NULL, 10) * 4096;
		if (size == 0) {
			size = 4096;
			if (errno) {
				perror("Invalid read size");
			}
		}
	}

	printf("Starting read test 3 with %d bytes...\n", size);
	fd = open("/dev/chardisk0", O_RDONLY);

	if (fd < 0) {
		perror("Failed to open the device");
		return errno;
	}

	printf("Reading from the device...\n");

	result = read(fd, buffer, size);

	printf("Result: %zd\n", result);

	if (result < 0) {
		perror("Failed to read from the device");
		return errno;
	}

//	outfd = open("readtest3-out", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
//	if (outfd < 0) {
//		perror("Failed to open output file");
//		return errno;
//	}
//	write(outfd, buffer, BUFFER_LENGTH);
//	close(outfd);

	return 0;
}
