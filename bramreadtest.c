#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
//#include <malloc.h>
#include <sys/ioctl.h>
#include <linux/types.h>

#include "chario.h"

//#define BUFFER_LENGTH 8192 // Should be a multiple of NVMe block size, as we DMA straight to it without checking
#define BRAM_SIZE 0x200000
#define BRAM_START 0x60000000

static char in_buf[BRAM_SIZE] __attribute__((aligned(0x1000)));
static char out_buf[BRAM_SIZE] __attribute__((aligned(0x1000)));

char *bram;
int mfd;

void cleanup() {
	printf("Cleaning up...\n");
	munmap(bram, BRAM_SIZE);
	close(mfd);
}

int main(void) {
	int fd, infd, outfd;
	ssize_t result;


//	char *buffer = malloc(BUFFER_LENGTH);
//	char *buffer = memalign(4096, BUFFER_LENGTH);

	printf("Starting read test...\n");

	mfd = open("/dev/mem", O_RDONLY | O_SYNC);
	if (mfd == -1) {
		fprintf(stderr, "Can't open /dev/mem");
		cleanup();
		return 1;
	}

	bram = mmap(NULL, BRAM_SIZE, PROT_READ, MAP_SHARED, mfd, BRAM_START);
	if (bram == MAP_FAILED) {
		perror("Can't map BRAM memory");
		cleanup();
		return 1;
	}

	fd = open("/dev/chardisk0", O_RDONLY);

	if (fd < 0) {
		perror("Failed to open the device");
		return errno;
	}

	printf("Reading from the device...\n");

	struct chario_phys_io io = {
		.address = BRAM_START,
		.length = BRAM_SIZE
	};

	result = ioctl(fd, CHARIO_IOCTL_READ_PHYS, &io);

	printf("Result: %zd\n", result);

	if (result < 0) {
		perror("Failed to read the message from the device");
		return errno;
	}

	lseek(fd, 0, SEEK_SET);
	read(fd, in_buf, BRAM_SIZE); // Read actual data from device
	memcpy(out_buf, bram, BRAM_SIZE); // Copy BRAM to buffer

	close(fd);

	munmap(bram, BRAM_SIZE);
	close(mfd);

	infd = open("bramreadtest-in", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
	if (infd < 0) {
		perror("Failed to open output file 1");
		return errno;
	}
	write(infd, in_buf, BRAM_SIZE);
	close(infd);

	outfd = open("bramreadtest-out", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
	if (outfd < 0) {
		perror("Failed to open output file 1");
		return errno;
	}
	write(outfd, out_buf, BRAM_SIZE);
	close(outfd);

//	free(buffer); // Freeing here gives a 'double free or corruption' error?

	return 0;
}
