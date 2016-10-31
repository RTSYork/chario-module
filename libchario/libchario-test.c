#include <stdio.h>
#include "libchario.h"


int main(int argc, char *argv[]) {

	printf("libchario test\n");

	chario_init_device();

	struct chario_task task;

	chario_init_task(&task, 0);

	chario_add_range_to_task(&task, 1, 0);
	chario_add_range_to_task(&task, 2, 50);
	chario_add_range_to_task(&task, 32, 100);

	chario_read_blocks_for_task(&task);

	return 0;
}
