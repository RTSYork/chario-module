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

//static char in_buf[DDR_SIZE/4] __attribute__((aligned(0x1000)));
//static char out_buf[DDR_SIZE/4] __attribute__((aligned(0x1000)));

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

	printf("Writing message to DDR...\n");

	mfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (mfd == -1) {
		fprintf(stderr, "Can't open /dev/mem");
		cleanup();
		return 1;
	}

	ddr = mmap(NULL, DDR_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mfd, DDR_START);
	if (ddr == MAP_FAILED) {
		perror("Can't map BRAM memory");
		cleanup();
		return 1;
	}

	sprintf(ddr, "Hello there. I am some mmapped DDR.");

	munmap(ddr, DDR_SIZE);
	close(mfd);

	return 0;
}
