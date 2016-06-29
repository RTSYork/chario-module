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

#include "charfs.h"

//#define BUFFER_LENGTH 8192 // Should be a multiple of NVMe block size, as we DMA straight to it without checking
#define OCM_SIZE 0x40000
#define OCM_START 0xFFFC0000

static char in_buf[OCM_SIZE] __attribute__((aligned(0x1000)));
static char out_buf[OCM_SIZE] __attribute__((aligned(0x1000)));

char *ocm;
int mfd;

void cleanup() {
	printf("Cleaning up...\n");
	munmap(ocm, OCM_SIZE);
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

	ocm = mmap(NULL, OCM_SIZE, PROT_READ, MAP_SHARED, mfd, OCM_START);
	if (ocm == MAP_FAILED) {
		perror("Can't map OCM memory");
		cleanup();
		return 1;
	}

	fd = open("/dev/charfs", O_RDWR);

	if (fd < 0) {
		perror("Failed to open the device");
		return errno;
	}

	printf("Reading from the device...\n");

	struct charfs_phys_io io = {
		.address = OCM_START,
		.length = OCM_SIZE
	};

	result = ioctl(fd, CHARFS_IOCTL_WRITE_PHYS, &io);

	printf("Result: %zd\n", result);

	if (result < 0) {
		perror("Failed to read the message from the device");
		return errno;
	}

	lseek(fd, 0, SEEK_SET);
	read(fd, out_buf, OCM_SIZE); // Read actual data from device
	memcpy(in_buf, ocm, OCM_SIZE); // Copy OCM to buffer

	close(fd);

	munmap(ocm, OCM_SIZE);
	close(mfd);

	infd = open("ocmwritetest-in", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
	if (infd < 0) {
		perror("Failed to open output file 1");
		return errno;
	}
	write(infd, in_buf, OCM_SIZE);
	close(infd);

	outfd = open("ocmwritetest-out", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
	if (outfd < 0) {
		perror("Failed to open output file 1");
		return errno;
	}
	write(outfd, out_buf, OCM_SIZE);
	close(outfd);

//	free(buffer); // Freeing here gives a 'double free or corruption' error?

	return 0;
}
