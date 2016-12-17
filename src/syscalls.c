#include "syscalls.h"


extern pcb_t *current_process;
extern devices_sem_t semaphores;
extern cputime_t last_ldst;
extern uint soft_block_c;


int swag = 0;

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


void init_handler(uint exc_index, uint flag, memaddr pc, memaddr sp, int cpsr) {
    if (current_process->p_flags & flag) {
        sys2_TERMINATEPROCESS(0, 0, 0);
    } else {
        // Set the sys6 called flag
        current_process->p_flags|= flag;

        state_t *new_area = &current_process->p_excpvec[exc_index];
        new_area->pc = pc;
        new_area->sp = sp;
        new_area->cpsr = cpsr;
        new_area->CP15_EntryHi = ENTRYHI_ASID_SET(new_area->CP15_EntryHi,
                ENTRYHI_ASID_GET(current_process->p_s.CP15_EntryHi));
    }
}

/*
 * createProcess (SYS1)
 * Create a new process, progeny of the caller
 */
void sys1_CREATEPROCESS(int a2, int a3, int a4) {
    static uint next_pid = 10;

    state_t* child_state = (state_t *) a2;
    pcb_t* child = (pcb_t *) allocPcb();
    if (child == NULL) {
        current_process->p_s.a1 = -1;
    } else {
        memcpy(&child->p_s, child_state, sizeof(state_t));
        current_process->p_s.a1 = child->p_pid = next_pid++;
        insertChild(current_process, child);
        insertProcQ(&ready_queue, child);
    }
}

/*
 * termProcess (SYS2)
 * Terminates a process and its progeny
 */
void sys2_TERMINATEPROCESS(int a2, int a3, int a4) {
    pid_t target_pid = a2;
    pcb_t *target = NULL;
    if (a4 == 666)
        swag = 1;

    // Suicide
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

/*
 * semaphoreOperation (SYS3)
 * this operation with positive values have the effect of freeing resources associated with the semaphore,
 * on the other hand, requesting the service with negative values allocates re-
 * sources.
 */
void sys3_SEMOP(int a2, int a3, int a4) {
    int *semaddr = (int *) a2;
    int weight = a3;

    *semaddr+= weight;

    if (weight == 0) {
        // Process is stupid. Kill all of his family. Kill them all.
        sys2_TERMINATEPROCESS(0, 0, 0);

    } else if(weight < 0) {

        if (*semaddr < 0) {
            // Lock process
            current_process->p_resources_needed = -weight;
            current_process->p_slice_time-= getTODLO() - last_ldst;
            insertBlocked(semaddr, current_process);
            current_process = NULL;

            if ((memaddr)semaddr >= (memaddr)&semaphores
                    && (memaddr)semaddr < (memaddr)&semaphores+sizeof(devices_sem_t))
                soft_block_c++;
        }

    } else if(weight > 0) {
        // Unlock processes
        pcb_t *p;
        while ((p = headBlocked(semaddr))) {

            if (weight >= p->p_resources_needed) {
                weight-= p->p_resources_needed;
                p->p_resources_needed = 0;
                outBlocked(p);
                if ((memaddr)semaddr >= (memaddr)&semaphores
                        && (memaddr)semaddr < (memaddr)&semaphores+sizeof(devices_sem_t))
                    soft_block_c--;
                insertProcQ(&ready_queue, p);
            } else {
                p->p_resources_needed-= weight;
                break;
            }
        }
    }
}

/*
 * Sys_BP_Handler (SYS4)
 * When this service is requested, the kernel prepares the System Call/Breakpoint
 * New Area of the calling process for exception pass-up.
 */
void sys4_SPECSYSHDL(int a2, int a3, int a4) {
    init_handler(EXCP_SYS_NEW, PCB_FLAG_SYS_4, a2, a3, a4);
}

/*
 * TLB_Handler (SYS5)
 * When this service is requested, the kernel prepares the TLB Exception
 * New Area of the calling process for exception pass-up.
 */
void sys5_SPECTLBHDL(int a2, int a3, int a4) {
    init_handler(EXCP_TLB_NEW, PCB_FLAG_TLB_5, a2, a3, a4);
}

/*
 * Trap_Handler (SYS6)
 * When this service is requested, the kernel prepares the TLB Exception
 * New Area of the calling process for exception pass-up.
 */
void sys6_SPECPGMTHDL(int a2, int a3, int a4) {
    init_handler(EXCP_PGMT_NEW, PCB_FLAG_PGT_6, a2, a3, a4);
}

/*
 * exitTrap (SYS7)
 * When this service is requested, the kernel prepares the TLB Exception
 * New Area of the calling process for exception pass-up.
 */
void sys7_EXITTRAP(int a2, int a3, int a4) {
    current_process->p_excpvec[current_process->p_s.a2].a1 = a3;
    memcpy(&current_process->p_s, &current_process->p_excpvec[current_process->p_s.a2], sizeof(state_t)); // save old state
}

/*
 * getCpuTime (SYS8)
 * Returns processor time (in ms) to the calling process
 */
void sys8_GETCPUTIME(int a2, int a3, int a4) {
    *((cputime_t *)current_process->p_s.a2) = current_process->p_usr_time+current_process->p_sys_time;
    *((cputime_t *)current_process->p_s.a3) = current_process->p_usr_time;
}

/*
 * waitForClock (SYS9)
 * This instruction locks the requesting process on the nucleus maintained pseudo-
 * clock timer semaphore. Each process resources_needed on this semaphore is unlocked
 * every 100 milliseconds automatically by the nucleus
 */
void sys9_WAITCLOCK(int a2, int a3, int a4) {
    sys3_SEMOP((int) &semaphores.pseudo_clock, -1, a4);
}


/*
 */
void sys10_IODEVOP(int a2, int a3, int a4) {
    uint cmd = a2;
    uint line = a3;
    uint devno = a4;
    int *device_sem;
    uint *device_com;

    if (line == INT_TERMINAL) {
        uint is_read = devno >> 31;
        devno&= 0x000000FF;

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

/*
 * getPID (SYS11)
 * Returns process ID of the calling process
 */
void sys11_GETPID(int a2, int a3, int a4) {
    current_process->p_s.a1 = current_process->p_pid;
}
