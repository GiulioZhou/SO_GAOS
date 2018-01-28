#include "initUProc.h"

// =========================== Global Variables ============================= //

// Pointer to the segment table array
segtbl_entry_t *seg_table = (segtbl_entry_t *) SEGTABLE_START;

extern kptbl_t kseg0_ptbl;
extern uptbl_t useg3_ptbl;
extern pcb_t *current_process;
uptbl_t useg2_ptbl[UPROCMAX];

int global_asid = D_ASID+1;

// ========================= The Init Process =============================== //

// This is the entry point for every created process
void boot_proc() {
	int asid = current_process->p_asid;
	memaddr vframe_first = ADDR_UPROC_EXC+asid*2*FRAME_SIZE;
	memaddr vframe_second = vframe_first+FRAME_SIZE;
	SYSCALL(SPECSYSHDL,  (memaddr)handle_exc_sysbp, (memaddr) vframe_first + LAST_WORD, STATUS_ALL_INT_ENABLE(STATUS_SYS_MODE));
	SYSCALL(SPECTLBHDL,  (memaddr)handle_exc_tlb,   (memaddr) vframe_second + LAST_WORD, STATUS_ALL_INT_ENABLE(STATUS_SYS_MODE));
	SYSCALL(SPECPGMTHDL, (memaddr)handle_exc_trap,  (memaddr) vframe_first + LAST_WORD, STATUS_ALL_INT_ENABLE(STATUS_SYS_MODE));
	// set up process' state
	state_t state = setUpState(state, STATUS_USER_MODE, 1, 1, asid);

	state.sp = USEG2BASE | (MAX_PAGES-1) << 12 | FRAME_SIZE-WORD_SIZE; //Last page of Useg2
	// ELF file format (the code frame) has the first instruction at address 0x54
	state.pc = USEG2BASE | 0x54;
	LDST(&state);
}

int readBlockFromTape(memaddr *dest) {
	int status, data1;
	dtpreg_t *dev = (dtpreg_t *) DEV_REG_ADDR(IL_TAPE, 0);
	dev->data0 = (memaddr) dest;
	status = SYSCALL(IODEVOP, DEV_TAPE_C_READBLK, IL_TAPE, 0);
	data1 = status >> 8;
	status = status & STATMASK;
	if (status != 1) {
		return -1;
	}
	return data1;
}

int createProcessFromBlocks(struct clist file_blocks) {
	int i;
	// Check file size
	if (clist_empty(file_blocks)) {
		fatal("Cannot create process from empty file\n");
	}

	int asid = global_asid++;

	// Initialize the useg2 page table
	uptbl_t *pgtbl = &useg2_ptbl[asid];
	pgtbl->header = (PAGE_TBL_MAGICNO << 24) | MAX_PAGES;
	for (i = 0; i < MAX_PAGES; i++) {
		swap_list_t *block;
		clist_pop(&file_blocks, block, next);
		pgtbl->entries[i].entry_hi = USEG2 << 30 | i << 12 | asid << 5;
		if (block) {
			pgtbl->entries[i].entry_lo = block->head << 24 | block->sect << 16;
		} else {
			pgtbl->entries[i].entry_lo = 0;
		}
	}

	// Init segment table pointers
	seg_table[asid].kseg0 = &kseg0_ptbl;
	seg_table[asid].useg2 = &useg2_ptbl[asid];
	seg_table[asid].useg3 = &useg3_ptbl;

	// Create the bootstrapping state
	memaddr frame_first = ADDR_UPROC_EXC+asid*2*FRAME_SIZE;
	state_t state = setUpState(state, STATUS_SYS_MODE, 1, 0, asid);
	state.sp = frame_first;
	state.pc = (memaddr) boot_proc;

	return SYSCALL(CREATEPROCESS, (int)&state, asid, 0);
}

// Reads one file from tape
int readFileFromTapeAndStartProcess() {
	char buffer[FRAME_SIZE];
	struct clist file_blocks = CLIST_INIT;
	int tape_status;
	swap_list_t *block;
	while (1) {
		// Read the next block
		tape_status = readBlockFromTape((memaddr *) buffer);
		if (tape_status == -1) {
			return 0;
		}
		// Swap the block in the disk
		block = frame_to_disk((memaddr *)buffer);
		clist_enqueue(block, &file_blocks, next);
		// Check if file finished
		if (tape_status == TAPE_EOF || tape_status == TAPE_EOT) {
			return createProcessFromBlocks(file_blocks);
		}
	}
}

// Creates u-procs and returns the number of u-procs created
int init_proc() {
	int processes = 0;
	while (readFileFromTapeAndStartProcess()) processes++;
	return processes;
}
