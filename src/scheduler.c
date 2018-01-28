#include "scheduler.h"

extern pcb_t *current_process;
extern uint process_c;
extern uint soft_block_c;
extern struct clist ready_queue;
extern cputime_t last_ldst;
extern uint next_pseudo_tick;
extern devices_sem_t semaphores;
pcb_t *init;

void scheduler() {
pcb_t *p;
void *tmp;

	// Check if the pseudo tick happened while in kernel mode with int disabled
	while (getTODLO() > next_pseudo_tick) {
		next_pseudo_tick+= SCHED_PSEUDO_CLOCK;
		if (semaphores.pseudo_clock < 0)
			sys3_SEMOP((int) &semaphores.pseudo_clock, -semaphores.pseudo_clock, 0);
	}

	if (!current_process) {
		current_process = removeProcQ(&ready_queue);
	}

	if (current_process) {
		// Process gets a shiny new time slice
		if (current_process->p_slice_time <= 0) {
			current_process->p_slice_time = SCHED_TIME_SLICE;
		}
		// Time slice might be [almost] finished, so skip the process
		if (current_process->p_slice_time <= 10) {
			current_process->p_slice_time = SCHED_TIME_SLICE;
			insertProcQ(&ready_queue, current_process);
			//tprintHex(current_process->p_asid);
			current_process = NULL;
			scheduler();
			fatal("Scheduler returned\n");
			PANIC();
		}

		// Next timer interrupt will be for the pseudo clock or for slice timeout
		uint timer = min(
			 current_process->p_slice_time,
			 next_pseudo_tick - getTODLO()
		 );

		setTIMER(timer);

		last_ldst = getTODLO();
		LDST(&current_process->p_s);

	} else if (clist_empty(ready_queue)) {
		if (process_c == 0) { // Processes finished
			disableInterrupts();
			tprint("All processes terminated\n");
			HALT();
		} else if (soft_block_c == 0) { // Error case
			fatal("No process ready but soft_block_c == 0\n");
			PANIC();
		} else { // Go idle
			setTIMER(next_pseudo_tick - getTODLO());
			setSTATUS(STATUS_ALL_INT_ENABLE(getSTATUS()));
			WAIT();
		}
	}
	p = headProcQ(&ready_queue);
	tprintHex(p->p_asid);
	fatal("removeProcQ returned NULL but ready_queue is not empty");
	PANIC();
}

void schedStart(pcb_t *pcb) {
	init = pcb;
	insertProcQ(&ready_queue, pcb);
	next_pseudo_tick = getTODLO()+SCHED_PSEUDO_CLOCK;
	scheduler();
}
