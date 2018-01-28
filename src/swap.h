#ifndef _SWAP_H
#define _SWAP_H

#include "const.h"
#include "types.h"
#include "clist.h"
#include "util.h"


void resetFrame(int index);
void initSwapPool();
swap_list_t *swap_alloc();
void swap_release(byte sect, byte head);
void disk_init();
int disk_io (memaddr addr, int disk_number, int disk_sector, int operation);
swap_list_t *frame_to_disk(memaddr frame_addr);
void disk_to_frame(byte head, byte sect, memaddr frame_addr);

#endif
