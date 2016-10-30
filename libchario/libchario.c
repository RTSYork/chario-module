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

int chario_add_range_to_task(struct chario_task *task, size_t size, off_t offset) {
	struct chario_blocks_range *range;
	struct chario_blocks_range *prev_item;

	dbg_print("chario_add_range_to_task(%d, %zu, %jd)", task->id, size, (intmax_t)offset);

	range = malloc(sizeof(struct chario_blocks_range));

	range->start = offset;
	range->count = size;

	if (!task->ranges) {
		range->buffer_offset = 0;
		INIT_LIST_HEAD(&range->list);
		task->ranges = range;
		dbg_print("  first");
		dbg_print("  task->ranges = 0x%08x", (unsigned)task->ranges);
	}
	else {
		prev_item = list_last_entry(&task->ranges->list, struct chario_blocks_range, list);
		range->buffer_offset = prev_item->buffer_offset + (prev_item->count * CHARIO_BLOCK_SIZE);
		list_add_tail(&range->list, &task->ranges->list);
		dbg_print("  not first");
		dbg_print("  task->ranges = 0x%08x", (unsigned)task->ranges);
	}

	dbg_print("  range->buffer_offset = %jd", (intmax_t)range->buffer_offset);

	return 0;
}

int chario_read_blocks_for_task(struct chario_task *task) {
	struct chario_blocks_range *range;
	int i = 0;

	dbg_print("chario_read_blocks_for_task(%d)", task->id);

	list_for_each_entry(range, &task->ranges->list, list) {
		dbg_print("  item %d", i++);
		dbg_print("  range = 0x%08x", (unsigned)range);
		dbg_print("    range->start = %jd", (intmax_t)range->start);
		dbg_print("    range->count = %zu", range->count);
		dbg_print("    range->buffer_offset = %jd", (intmax_t)range->buffer_offset);
	}

	return 0;
}
