#ifndef CHARIO_LIBCHARIO_H
#define CHARIO_LIBCHARIO_H

#include <stddef.h>
#include <sys/types.h>
#include "list.h"

#define LIBCHARIO_DBG

#define CHARIO_BLOCK_SIZE 4096


struct chario_task {
    int id;
    struct chario_blocks_range *ranges;
};

struct chario_blocks_range {
    struct list_head list;
    off_t start;
    size_t count;
    off_t buffer_offset;
};


int chario_init_device(void);

int chario_add_range_to_task(struct chario_task *task, size_t size, off_t offset);

int chario_read_blocks_for_task(struct chario_task *task);


#ifdef LIBCHARIO_DBG
#define dbg_print(format, ...) printf("libchario: " format "\n", ##__VA_ARGS__)
#else
#define dbg_print(format, args...) {}
#endif

#endif //CHARIO_LIBCHARIO_H
