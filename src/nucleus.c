#include "clist.h"
#include "pcb.h"
#include "asl.h"
#include "types.h"
#include "const.h"
#include "scheduler.h"
#include "exceptions.h"
#include <libuarm.h>

// Semaphores
pcb_t *current_process = NULL;
uint process_c = 0;
uint soft_block_c = 0;
struct clist ready_queue = CLIST_INIT;
devices_sem_t semaphores;
uint next_pseudo_tick;
cputime_t last_ldst;
cputime_t boot_start;

extern void init();

void initExceptionHandlers() {
	state_t handler;
	// Processor Status Register
	handler.cpsr = STATUS_SYS_MODE;
	handler.cpsr = STATUS_ALL_INT_DISABLE(handler.cpsr);
	handler.cpsr = STATUS_DISABLE_TIMER(handler.cpsr);
	// System Control Register
	handler.CP15_Control = CP15_CONTROL_NULL;
	// Other registers
	handler.sp = RAM_TOP;
	handler.pc = (memaddr) excHandleInterrupt;
	memcpy((state_t *) INT_NEWAREA, &handler, sizeof(state_t));
	handler.pc = (memaddr) excHandleTLB;
	memcpy((state_t *) TLB_NEWAREA, &handler, sizeof(state_t));
	handler.pc = (memaddr) excHandlePGMT;
	memcpy((state_t *) PGMTRAP_NEWAREA, &handler, sizeof(state_t));
	handler.pc = (memaddr) excHandleSYSBP;
	memcpy((state_t *) SYSBK_NEWAREA, &handler, sizeof(state_t));
}

pcb_t *makeInit() {
	// Init the first process
	pcb_t *pcb = allocPcb();
	pcb->p_pid = 1;
	// Processor Status Register
	pcb->p_s.cpsr = STATUS_SYS_MODE;
	pcb->p_s.cpsr = STATUS_ALL_INT_ENABLE(pcb->p_s.cpsr);
	pcb->p_s.cpsr = STATUS_ENABLE_TIMER(pcb->p_s.cpsr);
	// System Control Register
	pcb->p_s.CP15_Control = CP15_CONTROL_NULL;
	// Other registers
	pcb->p_s.pc = (memaddr) init;
	pcb->p_s.sp = RAM_TOP-FRAMESIZE;
	return pcb;
}


int main() {
	initExceptionHandlers();
	initPcbs();
	initASL();
	// Initialize device semaphores
	memset(&semaphores, 0, sizeof(semaphores));
	// Create init and start the scheduler
	pcb_t *init;
	init = makeInit();
	init->p_pid = 1;
	boot_start=getTODLO();
	schedStart(init);
	return 0;
}
