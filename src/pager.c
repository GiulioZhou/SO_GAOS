#include "pager.h"

extern segtbl_entry_t *seg_table;
extern kptbl_t kseg0_ptbl;
extern uptbl_t useg2_ptbl[UPROCMAX];
extern uptbl_t useg3_ptbl;

extern swap_pool_t swap_pool[SWAP_POOL_SIZE];
extern int swap_semaphore;
extern pcb_t *current_process;


// Return the first frame not occupied
int getFreeFrame() {
	static int vic = 0;
	int i;
	// Search free page
	for (i = 0; i < SWAP_POOL_SIZE; i++) {
		if (!swap_pool[i].pte) {
			return i;
		}
	}
	// Swap the vic frame
	memaddr victim_frame = ADDR_PAGE_POOL + vic * FRAME_SIZE;
	//Round robin
	vic = (vic+1) % (SWAP_POOL_SIZE);
	swap_list_t *block = frame_to_disk(victim_frame);

	disableInterrupts();
	resetFrame(vic);
	swap_pool[vic].pte->entry_lo = block->head << 24 | block->sect << 16;
	TLBCLR();
	enableInterrupts();

	return vic;
}

void pager(memaddr bad_addr) {
	byte seg_no = bad_addr >> 30;
	static int a = 0;
	unsigned int vpn = (bad_addr << 2) >> 14;

	if (!current_process) {
		disableInterrupts();
		fatal("Pager error: current process is NULL\n");
	}
	if (vpn >= MAX_PAGES) {
		disableInterrupts();
		fatal("Pager error: VPN too high\n");
	}

	ptentry_t *pte = NULL;
	byte asid = current_process->p_asid;

	switch (seg_no) {
		case KSEG0:
			fatal("Pager error: kseg0 TLB exception\n");
			break;
		case USEG2:
			if (!seg_table[asid].useg2) {
				disableInterrupts();
				fatal("Pager error: Useg2 page table pointer undefined");
			}
			pte = &seg_table[asid].useg2->entries[vpn];
			break;
		case USEG3:
			if (!seg_table[asid].useg3) {
				disableInterrupts();
				fatal("Pager error: Useg3 page table pointer undefined");
			}
			pte = &seg_table[asid].useg3->entries[vpn];
			break;
		default:
			fatal("Pager error: Seg_no unknown\n");
	}

	if (!pte) {
		disableInterrupts();
		fatal("Pager error: undefined page table entry");
	}

	// Get mutex
	SYSCALL(SEMOP, (int) &swap_semaphore, -1, 0);
	// If no longer missing
	if (pte->entry_lo & ENTRYLO_VALID) {
		SYSCALL(SEMOP, (int) &swap_semaphore, 1, 0);
		return;
	}
	// PTE is invalid
	int pfn = pte->entry_lo >> 12;
	int page_index = getFreeFrame();
	memaddr frame_addr = ADDR_PAGE_POOL + page_index*FRAME_SIZE;

	// Check if page was occupied
	if (pfn) {
		byte head = pfn >> 12;
		byte sect = pfn >> 4 & 0xFF;
		disk_to_frame(head, sect, frame_addr);
	}

	pte->entry_lo = frame_addr | ENTRYLO_VALID | ENTRYLO_DIRTY;
	if (seg_no == USEG3) {
		pte->entry_lo |= ENTRYLO_GLOBAL;
	}
	disableInterrupts();
	// Setup swap_pool frame
	swap_pool[page_index].pte = pte;
	swap_pool[page_index].asid = asid;
	swap_pool[page_index].vpn = vpn;
	swap_pool[page_index].seg_no = seg_no;
	TLBCLR();
	enableInterrupts();
	// Release mutex
	SYSCALL(SEMOP, (int) &swap_semaphore, 1, 0);
}
