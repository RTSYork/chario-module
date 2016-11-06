#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include "libchario.h"

struct chario_task task1;
struct chario_task task2;
struct chario_task task3;

struct chario_task *tasks[] = {&task1, &task2, &task3};

char *buffer;

int returnVal = 0;

int main(int argc __attribute__ ((unused)), char *argv[] __attribute__ ((unused))) {

	printf("\n** libchario test started **\n\n");

	buffer = chario_init_device();

	chario_init_task(&task1, 1);
	chario_add_range_to_task(&task1, 1, 0);
	chario_add_range_to_task(&task1, 2, 50);
	chario_add_range_to_task(&task1, 32, 100);

	chario_init_task(&task2, 2);
	chario_add_range_to_task(&task2, 131072, 100000);

	chario_init_task(&task3, 3);
	chario_add_range_to_task(&task3, 65536, 256);
	chario_add_range_to_task(&task3, 32, 300000);

	for (uint8_t j = 0; j < 3; j++) {
		for (size_t k = 0; k < tasks[j]->total_size; k++) {
			buffer[k] = j << 4;
		}
		chario_flush_blocks_for_task(tasks[j]);
	}


	printf("\n** Tasks initialised **\n\n");


	for (uint8_t i = 0; i < 10; i++) {
		printf("\n** Iteration %d **\n\n", i);

		for (uint8_t j = 0; j < 3; j++) {
			chario_load_blocks_for_task(tasks[j]);

			for (size_t k = 0; k < tasks[j]->total_size; k++) {
				if (buffer[k] == (j << 4 | i)) {
					buffer[k]++;
				}
				else {
					printf("Byte 0x%08x is incorrect (0x%02x)\n", (unsigned)&buffer[k], buffer[k]);
					returnVal = 1;
					goto end;
				}
			}

			chario_flush_blocks_for_task(tasks[j]);
		}
	}


end:
	chario_close_device();

	return returnVal;
}
