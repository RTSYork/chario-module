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
#define DDR_SIZE 0x20000000
#define DDR_START 0x20000000

static char in_buf[DDR_SIZE/4] __attribute__((aligned(0x1000)));
static char out_buf[DDR_SIZE/4] __attribute__((aligned(0x1000)));

char *ddr;
int mfd;

void cleanup() {
	printf("Cleaning up...\n");
	munmap(ddr, DDR_SIZE);
	close(mfd);
}

int main(void) {
	int fd, infd, outfd;
	ssize_t result;


//	char *buffer = malloc(BUFFER_LENGTH);
//	char *buffer = memalign(4096, BUFFER_LENGTH);

	printf("Starting read test...\n");

	mfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (mfd == -1) {
		fprintf(stderr, "Can't open /dev/mem");
		cleanup();
		return 1;
	}

	ddr = mmap(NULL, DDR_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mfd, DDR_START);
	if (ddr == MAP_FAILED) {
		perror("Can't map DDR memory");
		cleanup();
		return 1;
	}

	fd = open("/dev/chardisk0", O_RDWR);

	if (fd < 0) {
		perror("Failed to open the device");
		return errno;
	}

	printf("Writing to the device...\n");

	struct chario_phys_io io = {
		.address = DDR_START,
		.length = DDR_SIZE
	};

	result = ioctl(fd, CHARIO_IOCTL_WRITE_PHYS, &io);

	printf("Result: %zd\n", result);

	if (result < 0) {
		perror("Failed to read the message from the device");
		return errno;
	}

	lseek(fd, 0, SEEK_SET);
	read(fd, out_buf, DDR_SIZE/4); // Read actual data from device
	memcpy(in_buf, ddr, DDR_SIZE/4); // Copy DDR to buffer

	close(fd);

	munmap(ddr, DDR_SIZE);
	close(mfd);

	infd = open("ddrwritetest-in", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
	if (infd < 0) {
		perror("Failed to open output file 1");
		return errno;
	}
	write(infd, in_buf, DDR_SIZE/4);
	close(infd);

	outfd = open("ddrwritetest-out", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
	if (outfd < 0) {
		perror("Failed to open output file 1");
		return errno;
	}
	write(outfd, out_buf, DDR_SIZE/4);
	close(outfd);

//	free(buffer); // Freeing here gives a 'double free or corruption' error?

	return 0;
}
