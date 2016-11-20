#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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
//#define DDR_SIZE 0x40000
//#define DDR_START 0xFFFC0000

static char out_buf[0x20000] __attribute__((aligned(0x1000)));

struct chario_phys_io io = {
	.address = DDR_START,
	.length = 0x20000
};

uint8_t *ddr;
int mfd;

void cleanup() {
	printf("Cleaning up...\n");
	munmap(ddr, DDR_SIZE);
	close(mfd);
}

int main(void) {
	int fd, outfd;
	ssize_t result;

	printf("Starting ACP test...\n");

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



	printf("Writing 0x00 to DDR...\n");
	for (int i = 0; i < 0x20000; i++) {
		ddr[i] = 0x00;
	}
//	msync(ddr, 0x20000, MS_SYNC | MS_INVALIDATE);

	printf("Writing 0xAB and 0xCD to DDR...\n");
//	for (int i = 0; i < 0x1000; i++) {
//		ddr[i] = 0xAC;
//	}
//	for (int i = 0x1000; i < 0x2000; i++) {
//		ddr[i] = 0xBD;
//	}
	for (int i = 0; i < 0x20000; i++) {
		ddr[i] = (uint8_t)i;
	}
//	msync(ddr, 0x20000, MS_SYNC | MS_INVALIDATE);

	printf("Writing acptest-out-1...\n");
	memcpy(out_buf, ddr, 0x20000);
	outfd = open("acptest-out-1", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
	if (outfd < 0) {
		perror("Failed to open output file 1");
		return errno;
	}
	write(outfd, out_buf, 0x20000);
	close(outfd);

//	printf("Smashing the cache with 0x55...\n");
//	for (int i = 0x2000; i < 0x1000000; i++) {
//		ddr[i] = 0x55;
//	}
//	msync(ddr + 0x2000, 0x1000000, MS_SYNC | MS_INVALIDATE);

	printf("Writing block to SSD with ioctl()... ");
	lseek(fd, 0, SEEK_SET);
	result = ioctl(fd, CHARIO_IOCTL_WRITE_PHYS, &io);
	printf("Result: %zd\n", result);
	if (result < 0) {
		perror("Failed to read the message from the device");
		return errno;
	}

	printf("Writing 0x11 and 0x22 to DDR...\n");
	for (int i = 0; i < 0x10000; i++) {
		ddr[i] = 0x11;
	}
	for (int i = 0x10000; i < 0x20000; i++) {
		ddr[i] = 0x22;
	}
//	msync(ddr, 0x20000, MS_SYNC | MS_INVALIDATE);

	printf("Writing acptest-out-2...\n");
	memcpy(out_buf, ddr, 0x20000);
	outfd = open("acptest-out-2", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
	if (outfd < 0) {
		perror("Failed to open output file");
		return errno;
	}
	write(outfd, out_buf, 0x20000);
	close(outfd);

//	printf("Smashing the cache with 0x66...\n");
//	for (int i = 0x2000; i < 0x1000000; i++) {
//		ddr[i] = 0x66;
//	}
//	msync(ddr + 0x2000, 0x1000000, MS_SYNC | MS_INVALIDATE);

	printf("Reading block from SSD with ioctl()... ");
	lseek(fd, 0, SEEK_SET);
	result = ioctl(fd, CHARIO_IOCTL_READ_PHYS, &io);
	printf("Result: %zd\n", result);
	if (result < 0) {
		perror("Failed to read the message from the device");
		return errno;
	}

	printf("Writing acptest-out-3...\n");
	memcpy(out_buf, ddr, 0x20000);
	outfd = open("acptest-out-3", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
	if (outfd < 0) {
		perror("Failed to open output file");
		return errno;
	}
	write(outfd, out_buf, 0x20000);
	close(outfd);

	printf("Reading block from SSD with read()... ");
	lseek(fd, 0, SEEK_SET);
	result = read(fd, out_buf, 0x20000);
	printf("Result: %zd\n", result);
	if (result < 0) {
		perror("Failed to read the message from the device");
		return errno;
	}

	printf("Writing acptest-out-4...\n");
	outfd = open("acptest-out-4", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
	if (outfd < 0) {
		perror("Failed to open output file");
		return errno;
	}
	write(outfd, out_buf, 0x20000);
	close(outfd);



	printf("Writing 0xEF and 0x89 to DDR...\n");
	for (int i = 0; i < 0x10000; i++) {
		ddr[i] = 0xEF;
	}
	for (int i = 0x10000; i < 0x20000; i++) {
		ddr[i] = 0x89;
	}
//	msync(ddr, 0x20000, MS_SYNC | MS_INVALIDATE);

	memcpy(out_buf, ddr, 0x20000);

	printf("Writing acptest-out-5...\n");
	outfd = open("acptest-out-5", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
	if (outfd < 0) {
		perror("Failed to open output file");
		return errno;
	}
	write(outfd, out_buf, 0x20000);
	close(outfd);

	printf("Writing block to SSD with write()... ");

	lseek(fd, 0, SEEK_SET);
	result = write(fd, out_buf, 0x20000);
	printf("Result: %zd\n", result);
	if (result < 0) {
		perror("Failed to read the message from the device");
		return errno;
	}

	printf("Reading block from SSD with read()... ");
	lseek(fd, 0, SEEK_SET);
	result = read(fd, out_buf, 0x20000);
	printf("Result: %zd\n", result);
	if (result < 0) {
		perror("Failed to read the message from the device");
		return errno;
	}

	printf("Writing acptest-out-6...\n");
	outfd = open("acptest-out-6", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
	if (outfd < 0) {
		perror("Failed to open output file");
		return errno;
	}
	write(outfd, out_buf, 0x20000);
	close(outfd);



	close(fd);
	munmap(ddr, DDR_SIZE);
	close(mfd);

	return 0;
}
