#include "const.h"
#include "syscalls.h"
#include "types.h"
#include "util.h"
#include <arch.h>
#include "swap.h"


void pager(memaddr bad_addr);
swap_list_t *swap_alloc();
void disk_init();
void initSwapPool();
swap_list_t *frame_to_disk(memaddr frame_addr);
