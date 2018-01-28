#include "swap.h"

swap_pool_t swap_pool[SWAP_POOL_SIZE]; //Physical frames for paging purposes

static swap_list_t disk_free_list[MAX_HEAD][MAX_SECT]; //Swap area -> secondary memory in which pages are stored
struct clist swap_free = CLIST_INIT; //Not used sectors in the secondary memory

//----------SWAP POOL---------------------

void resetFrame(int index) {
	swap_pool[index].pte = NULL;
	swap_pool[index].asid = 0;
	swap_pool[index].seg_no = 0;
	swap_pool[index].vpn = 0;
}

void initSwapPool() {
	int i;
	for (i = 0; i < SWAP_POOL_SIZE; i++) {
		resetFrame(i);
	}
}

//Returns a free disk sector
swap_list_t *swap_alloc() {
	swap_list_t *ret;
	clist_pop(&swap_free, ret, next);
	return ret;
}

void swap_release(byte sect, byte head) {
	if (disk_free_list[head][sect].next.next != NULL) {
		return;
	}
	disk_free_list[head][sect].head = head;
	disk_free_list[head][sect].sect = sect;
	clist_enqueue(&disk_free_list[head][sect], &swap_free, next);
}


//-----------SECONDARY MEMORY-----------------------

void disk_init() {
	int i, j;
	for(i=0; i < MAX_HEAD; i++) {
		for(j=1; j < MAX_SECT; j++) {
			disk_free_list[i][j].head=i;
			disk_free_list[i][j].sect=j;
			disk_free_list[i][j].next = CLIST_INIT;
			clist_enqueue(&disk_free_list[i][j], &swap_free, next);
		}
	}
}

// I/O operations on a device sector
int disk_io (memaddr addr, int disk_number, int disk_sector, int operation){
	static int disk_sem = 1;
	unsigned int status;
	int cmd;
	dtpreg_t *dev;

	cmd = operation | disk_sector << 8;
	dev = (dtpreg_t *) DEV_REG_ADDR(IL_DISK, disk_number);
	SYSCALL(SEMOP, (memaddr)&disk_sem, -1, 0);
	dev->data0 = addr;
	status = SYSCALL(IODEVOP, cmd, IL_DISK, disk_number);
	SYSCALL(SEMOP, (memaddr)&disk_sem, 1, 0);
	status = status & 0xFF;
	return status;
}

// Put a frame in the backing store device
swap_list_t *frame_to_disk(memaddr frame_addr) {
	swap_list_t *block = swap_alloc();
	int status, data1;
	if (!block) {
		fatal("No more free blocks for swap");
	}
	status = disk_io(frame_addr, 0, block->head << 8 | block->sect , DEV_DISK_C_WRITEBLK);
	data1 = status >> 8;
	status = status & STATMASK;
	if (status != 1) {
		fatal("frame_to_disk disk_io failed ");
	}
	return block;
}

// Put a disk frame in the memory
void disk_to_frame(byte head, byte sect, memaddr frame_addr) {
	// Read the disk frame in the memory
	int status = disk_io(frame_addr, 0, head << 8 | sect , DEV_DISK_C_READBLK);
	int data1 = status >> 8;
	status = status & STATMASK;
	if (status != 1) {
		fatal("disk_to_frame disk_io failed ");
	}
	swap_release(head, sect);
}
