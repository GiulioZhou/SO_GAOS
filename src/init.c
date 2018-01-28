#include "const.h"
#include "syscalls.h"
#include "types.h"
#include "pager.h"
#include "passups.h"
#include "delay.h"
#include "initUProc.h"
#include <arch.h>
#include "util.h"

extern pcb_t *current_process;
// =========================== Global Variables ============================= //

kptbl_t kseg0_ptbl;
uptbl_t useg3_ptbl;

uint uprocess_c = 0;

// Semaphores
int uproc_semaphore[UPROCMAX];
int swap_semaphore;
int master_sem = 0;

// =================== Page Tables Inits ==================================== //

// Initialize the kernel page table
void init_kernel_pt() {
	int i;
	kseg0_ptbl.header = (PAGE_TBL_MAGICNO << 24) | MAX_KPAGES;
	for (i = 0; i < MAX_KPAGES; i++) {
		kseg0_ptbl.entries[i].entry_hi = i << 12;
		kseg0_ptbl.entries[i].entry_lo = i << 12 | ENTRYLO_DIRTY | ENTRYLO_VALID | ENTRYLO_GLOBAL;
	}
}

// Initialize the useg3 page table
void init_useg3_pt() {
	int i;
	useg3_ptbl.header = (PAGE_TBL_MAGICNO << 24) | MAX_PAGES;
	for (i = 0; i < MAX_PAGES; i++) {
		useg3_ptbl.entries[i].entry_hi = USEG3 << 30 | i << 12;
		useg3_ptbl.entries[i].entry_lo = 0 | ENTRYLO_GLOBAL;
	}
}

// =================== The Init process ==================================== //

void init() {
	int i;
	// Initialize kseg0 & useg3 page table (same for all processes)
	init_kernel_pt();
	init_useg3_pt();

	// Initilize all VM-I/O support level semaphores
	swap_semaphore = 1;
	memset(uproc_semaphore, 0, sizeof(int) * UPROCMAX); //Private semaphore for delay facility

	initADL();
	init_del_deamon();
	initAVSL();
	disk_init();
	initSwapPool();
	initDMA();

	// Init all the u-procs from tape and get the number of u-procs created
	uprocess_c = init_proc();
	// wait for all uprocs to terminate
	SYSCALL(SEMOP,&master_sem,-1,0);
	//Terminate Deamon
	SYSCALL(TERMINATEPROCESS,3,0,0);

	//Terminate the init process (and all his children)
	//This should HALT the system
	SYSCALL(TERMINATEPROCESS,0,0,0);
	fatal("System failed to terminate");
}
