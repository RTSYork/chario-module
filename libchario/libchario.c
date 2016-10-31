#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "libchario.h"

int fd;


int chario_init_device(void) {
	fd = open("/dev/chardisk0", O_RDWR);

	dbg_print("chario_init_device");

	return 0;
}


int chario_init_task(struct chario_task *task, int id) {
	dbg_print("chario_init_task(0x%08x, %d)", (unsigned)task, id);
	task->id = id;
	INIT_LIST_HEAD(&task->ranges);
	return 0;
}


int chario_add_range_to_task(struct chario_task *task, size_t size, off_t offset) {
	struct chario_blocks_range *range;
	struct chario_blocks_range *prev_item;

	dbg_print("chario_add_range_to_task(%d, %zu, %jd)", task->id, size, (intmax_t)offset);

	range = malloc(sizeof(struct chario_blocks_range));

	range->start = offset;
	range->count = size;

	if (list_empty(&task->ranges)) {
		range->buffer_offset = 0;
		dbg_print("    first item");
	}
	else {
		prev_item = list_last_entry(&task->ranges, struct chario_blocks_range, list);
		range->buffer_offset = (off_t)(prev_item->buffer_offset + (prev_item->count * CHARIO_BLOCK_SIZE));
		dbg_print("    not first item");
	}

	list_add_tail(&range->list, &task->ranges);

	dbg_print("    range->buffer_offset = %jd", (intmax_t)range->buffer_offset);

	return 0;
}


#ifdef __clang__
#pragma clang diagnostic push
#pragma ide diagnostic ignored "CannotResolve"
#endif
int chario_read_blocks_for_task(struct chario_task *task) {
	struct chario_blocks_range *range;
	int i = 0;

	dbg_print("chario_read_blocks_for_task(%d)", task->id);

	list_for_each_entry(range, &task->ranges, list) {
		dbg_print("    item %d", i++);
		dbg_print("    range = 0x%08x", (unsigned)range);
		dbg_print("        range->start = %jd", (intmax_t)range->start);
		dbg_print("        range->count = %zu", range->count);
		dbg_print("        range->buffer_offset = %jd", (intmax_t)range->buffer_offset);
	}

	return 0;
}
#ifdef __clang__
#pragma clang diagnostic pop
#endif
