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
#define BRAM_SIZE 0x300000
#define BRAM_START 0xa0000000

//static char in_buf[BRAM_SIZE] __attribute__((aligned(0x1000)));
//static char out_buf[BRAM_SIZE] __attribute__((aligned(0x1000)));

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

	printf("Writing message to BRAM...\n");

	mfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (mfd == -1) {
		fprintf(stderr, "Can't open /dev/mem");
		cleanup();
		return 1;
	}

	bram = mmap(NULL, BRAM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mfd, BRAM_START);
	if (bram == MAP_FAILED) {
		perror("Can't map BRAM memory");
		cleanup();
		return 1;
	}

//	sprintf(bram, "Hello there. I am some BRAM."); // 29
	sprintf(bram, "Hello there. I am some BRAM...."); // 32
//	sprintf(bram, "Hello there. I am some mmapped DDR."); // 36

	munmap(bram, BRAM_SIZE);
	close(mfd);

	return 0;
}
