#include "exceptions.h"

extern devices_sem_t semaphores;
extern uint next_pseudo_tick;
extern pcb_t *current_process;
extern cputime_t last_ldst;

static void (* sc_callbacks[])() = {
	sys1_CREATEPROCESS,
	sys2_TERMINATEPROCESS,
	sys3_SEMOP,
	sys4_SPECSYSHDL,
	sys5_SPECTLBHDL,
	sys6_SPECPGMTHDL,
	sys7_EXITTRAP,
	sys8_GETCPUTIME,
	sys9_WAITCLOCK,
	sys10_IODEVOP,
	sys11_GETPID
};

void update_usr_time(pcb_t *p, cputime_t exc_start) {
	if (p && p->p_pid) {
		p->p_usr_time+= exc_start - last_ldst;
		p->p_slice_time-= exc_start - last_ldst;
	}
}

void update_sys_time(pcb_t *p, cputime_t exc_start) {
	if (p && p->p_pid) {
		p->p_sys_time+= getTODLO() - exc_start;
		p->p_slice_time-= getTODLO() - exc_start;
	}
}


void passup(state_t *old_area, uint exc_id, uint exc_old_id, uint flag) {
	// If the callback is set, call it, otherwise kill the process
	if (current_process->p_flags & flag) {

		memaddr bad_addr = getBadVAddr();

		uint cause = CAUSE_EXCCODE_GET(old_area->CP15_Cause);

		// Move the old area state into the exception vector old area
		memcpy(&current_process->p_excpvec[exc_old_id], old_area,
			   sizeof(state_t));

		// Set the cause to a1
		current_process->p_excpvec[exc_id].a1 = cause;
		current_process->p_excpvec[exc_id].a2 = bad_addr;

		// Move the callback state to the current process state
		memcpy(&current_process->p_s, &current_process->p_excpvec[exc_id],
			   sizeof(state_t));

	} else {
		tprint("Exception handler hasn't been initialized\n");
		sys2_TERMINATEPROCESS(0, 0, 0);
	}
}



void excHandleTLB() {
	cputime_t exc_start = getTODLO();
	update_usr_time(current_process, exc_start);

	passup((state_t *) TLB_OLDAREA, EXCP_TLB_NEW, EXCP_TLB_OLD, PCB_FLAG_TLB_5);

	update_sys_time(current_process, exc_start);
	scheduler();
}

void excHandlePGMT() {
	cputime_t exc_start = getTODLO();
	update_usr_time(current_process, exc_start);

	passup((state_t *) PGMTRAP_OLDAREA, EXCP_PGMT_NEW, EXCP_PGMT_OLD, PCB_FLAG_PGT_6);

	update_sys_time(current_process, exc_start);
	scheduler();
}

void passup_sysbp() {
	passup((state_t *) SYSBK_OLDAREA, EXCP_SYS_NEW, EXCP_SYS_OLD, PCB_FLAG_SYS_4);
}

void excHandleSYSBP() {
	cputime_t exc_start = getTODLO();
	update_usr_time(current_process, exc_start);

	// Update current process state
	memcpy(&current_process->p_s, (void *) SYSBK_OLDAREA, sizeof(state_t));

	// Get the syscall number
	int syscall = current_process->p_s.a1;

	// Save the current process for later
	pcb_t *proc = current_process;

	// Get the cause and handle the syscall
	uint cause = CAUSE_EXCCODE_GET(current_process->p_s.CP15_Cause);
	state_t ps = current_process->p_s;

	switch (cause) {
		case EXC_SYSCALL:
			if (syscall < 1) {
				// Invali syscall
				tprint("Invalid syscall\n");
				sys2_TERMINATEPROCESS(0, 0, 0);

			} else if (syscall > SYSCALL_MAX) {
				passup_sysbp();

			} else if ((current_process->p_s.cpsr & 0x1F) == STATUS_USER_MODE) {

				// User Mode cannot call these syscalls, so we throw a pgmtrap exception
				state_t *old_pgmtrap = (state_t *) PGMTRAP_OLDAREA;
				state_t *old_sysbp = (state_t *) SYSBK_OLDAREA;
				memcpy(old_pgmtrap, old_sysbp, sizeof(state_t));
				old_pgmtrap->CP15_Cause = CAUSE_EXCCODE_SET(old_pgmtrap->CP15_Cause, EXC_RESERVEDINSTR);

				passup((state_t *) PGMTRAP_OLDAREA, EXCP_PGMT_NEW, EXCP_PGMT_OLD, PCB_FLAG_PGT_6);

			} else {
				sc_callbacks[syscall-1](ps.a2, ps.a3, ps.a4);
			}
			break;
		case EXC_BREAKPOINT:
			passup_sysbp();
			break;
		default:
			tprint("Unknown exception\n");
			PANIC();
	}
	update_sys_time(proc, exc_start);
	scheduler();
}

void excHandleInterrupt() {
	cputime_t exc_start = getTODLO();
	update_usr_time(current_process, exc_start);

	if (current_process) {
		// Update current process state
		memcpy(&current_process->p_s, (void *) INT_OLDAREA, sizeof(state_t));
		current_process->p_s.pc-= 4;
	}


	uint cause = getCAUSE();

	if (CAUSE_IP_GET(cause, IL_CPUTIMER)) ;
	if (CAUSE_IP_GET(cause, IL_IPI))      ;
	if (CAUSE_IP_GET(cause, IL_TIMER))    handleTimer();
	if (CAUSE_IP_GET(cause, IL_DISK))     handleDevice(IL_DISK, semaphores.dev_disk);
	if (CAUSE_IP_GET(cause, IL_TAPE))     handleDevice(IL_TAPE, semaphores.dev_tape);
	if (CAUSE_IP_GET(cause, IL_ETHERNET)) ;
	if (CAUSE_IP_GET(cause, IL_PRINTER))  handleDevice(IL_PRINTER, semaphores.dev_printer);
	if (CAUSE_IP_GET(cause, IL_TERMINAL)) handleTerminal();

	// We do not call update_sys_time because this exc isn't throwed by the proc
	scheduler();
}
