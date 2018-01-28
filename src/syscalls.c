#include "syscalls.h"

extern devices_sem_t semaphores;
extern uint soft_block_c;
extern pcb_t *current_process;
extern int s_pseudo_clock;

void terminateFamily(pcb_t *p) {
	pcb_t *child;
	void *tmp;

	while (!emptyChild(p)) {
		child = removeChild(p);
		terminateFamily(child);
	}

	outChild(p);
	outProcQ(&ready_queue, p);
	if (p->p_cursem) {
		*p->p_cursem->s_semAdd+= p->p_resources_needed;
		outBlocked(p);
	}
	freePcb(p);
}


void init_handler(uint exc_index, uint flag, memaddr pc, memaddr sp, int flags) {
	if (current_process->p_flags & flag) {
		sys2_TERMINATEPROCESS(0, 0, 0);
	} else {

		current_process->p_flags |= flag;
		state_t *new_area = &current_process->p_excpvec[exc_index];
		memset(new_area, 0, sizeof(state_t));
		new_area->pc = pc;
		new_area->sp = sp;
		new_area->cpsr = STATUS_ALL_INT_ENABLE(STATUS_SYS_MODE);
		new_area->CP15_Control = CP15_VM_ON;
		new_area->CP15_EntryHi = current_process->p_asid << 5;
	}
}



void sys1_CREATEPROCESS(int a2, int a3, int a4) {
	static uint next_pid = 3;
	state_t* child_state = (state_t *) a2;
	pcb_t* child = (pcb_t *) allocPcb();
	if (child == NULL) {
		current_process->p_s.a1 = -1;
	}
	else {
		child->p_asid = a3;
		memcpy(&child->p_s, child_state, sizeof(state_t));
		current_process->p_s.a1 = child->p_pid = next_pid++;
		insertChild(current_process, child);
		insertProcQ(&ready_queue, child);
	}
}


void sys2_TERMINATEPROCESS(int a2, int a3, int a4) {
	pid_t target_pid = a2;
	pcb_t *target = NULL;

	if (!target_pid)
		target = current_process;
	else
		target = findPcb(target_pid);

	if (target) {
		terminateFamily(target);
		if (current_process != target) {
			current_process->p_s.a1 = 1;
		}
	} else {
		current_process->p_s.a1 = 0;
	}

	if (target == current_process)
		current_process = NULL;
}


//V operation for postive weight, P operation for negative weight
void sys3_SEMOP(int a2, int a3, int a4) {
	int *semaddr = (int *) a2;
	byte virt = ((memaddr) a2 >> 30) == USEG3;
	int weight = a3;
	int new_value;

	//If the semaphore is virtual the weight is modified elsewhere
	//new_value contains the updated semaphore value
	if (virt) new_value = a4;
	else new_value = (*semaddr+= weight);

	if (weight == 0) {
		// Process is stupid. Kill all of his family. Kill them all.
		sys2_TERMINATEPROCESS(0, 0, 0);

	} else if(weight < 0 && new_value < 0) {
			// Lock process (only when there are no more resources)
			current_process->p_resources_needed = -weight;
			insertBlocked(semaddr, current_process, virt);
			current_process = NULL;

			//Check if the semaphore is a device semaphore
			if ((memaddr) semaddr >= (memaddr) &semaphores && (memaddr) semaddr < (memaddr) &semaphores + sizeof(devices_sem_t))
				soft_block_c++;

	} else if(weight > 0) {
		// Unlock processes
		pcb_t *p;
		while ((p = headBlocked(semaddr, virt))) {

			if (weight >= p->p_resources_needed) {
				weight-= p->p_resources_needed;
				p->p_resources_needed = 0;
				removeBlocked(semaddr, virt);

				if ((memaddr)semaddr >= (memaddr)&semaphores && (memaddr)semaddr < (memaddr)&semaphores+sizeof(devices_sem_t))
					soft_block_c--;

				insertProcQ(&ready_queue, p);
			} else {
				p->p_resources_needed-= weight;
				break;
			}
		}
	}
}


//Prepares the New Areas for exception pass-up
void sys4_SPECSYSHDL(int a2, int a3, int a4) {
	init_handler(EXCP_SYS_NEW, PCB_FLAG_SYS_4, a2, a3, a4);
}

void sys5_SPECTLBHDL(int a2, int a3, int a4) {
	init_handler(EXCP_TLB_NEW, PCB_FLAG_TLB_5, a2, a3, a4);
}

void sys6_SPECPGMTHDL(int a2, int a3, int a4) {
	init_handler(EXCP_PGMT_NEW, PCB_FLAG_PGT_6, a2, a3, a4);
}

//Restores the oldarea after exiting a trap
void sys7_EXITTRAP(int cause, int ret, int a4) {
	state_t *oldarea;
	switch (cause) {
		case PCB_FLAG_SYS_4:
			oldarea = &current_process->p_excpvec[EXCP_SYS_OLD];
			oldarea->a1 = ret;
			break;
		case PCB_FLAG_TLB_5:
			oldarea = &current_process->p_excpvec[EXCP_TLB_OLD];
			break;
		case PCB_FLAG_PGT_6:
			oldarea = &current_process->p_excpvec[EXCP_PGMT_OLD];
			oldarea->a1 = ret;
			break;
		default:
			fatal("Invalid exit trap value\n");
			PANIC();
	}

	memcpy(&current_process->p_s, oldarea, sizeof(state_t)); // restore the old state
}


void sys8_GETCPUTIME(int a2, int a3, int a4) {
	*((cputime_t *)current_process->p_s.a2) = current_process->p_usr_time+current_process->p_sys_time;
	*((cputime_t *)current_process->p_s.a3) = current_process->p_usr_time;
}

//Wait for pseudo_clock timer
void sys9_WAITCLOCK(int a2, int a3, int a4) {
	sys3_SEMOP((int) &semaphores.pseudo_clock, -1, a4);
}


void sys10_IODEVOP(int cmd, int line, int devno) {
	int *device_sem;
	uint *device_com;

	if (line == INT_TERMINAL) {
		uint is_read = devno >> 31;
		devno&= 0xFF;

		termreg_t *reg = (termreg_t *) DEV_REG_ADDR(line, devno);

		if (is_read) {
			device_com = &reg->recv_command;
			device_sem = &semaphores.dev_term_rcv[devno];
		} else {
			device_com = &reg->transm_command;
			device_sem = &semaphores.dev_term_trs[devno];
		}

	} else {
		switch (line) {
			case INT_DISK:
				device_sem = &semaphores.dev_disk[devno];
				break;
			case INT_TAPE:
				device_sem = &semaphores.dev_tape[devno];
				break;
			case INT_UNUSED: // Not implemented
				disableInterrupts();
				fatal("Device not supported");
				PANIC();
				break;
			case INT_PRINTER:
				device_sem = &semaphores.dev_printer[devno];
				break;
		}
		dtpreg_t *reg = (dtpreg_t *) DEV_REG_ADDR(line, devno);
		device_com = &reg->command;
	}

	// Send the command to the device
	*device_com = cmd;

	// Lock the process
	sys3_SEMOP((int) device_sem, -1, 0);
}


void sys11_GETPID(int a2, int a3, int a4) {
	current_process->p_s.a1 = current_process->p_pid;
}
