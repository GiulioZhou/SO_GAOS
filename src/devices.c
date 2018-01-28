#include "devices.h"

extern devices_sem_t semaphores;
extern uint next_pseudo_tick;
extern pcb_t *current_process;
extern cputime_t last_ldst;
extern struct clist ready_queue;


byte firstActiveDevice(byte bitmap) {
	uint n;
	for (n = 0; n < 8; n++, bitmap/= 2) {
		if (bitmap & 1) return n;
	}
	return -1;
}


void handleTimer() {
	uint tod = getTODLO();

	if (tod > next_pseudo_tick) {
		// Pseudo-clock
		next_pseudo_tick+= SCHED_PSEUDO_CLOCK;

		if (semaphores.pseudo_clock < 0)
			sys3_SEMOP((int) &semaphores.pseudo_clock, -semaphores.pseudo_clock, 0);
	}
}


void handleTerminal() {
	memaddr *line = (memaddr *) CDEV_BITMAP_ADDR(IL_TERMINAL);
	int devno = firstActiveDevice(*line);
	termreg_t *reg = (termreg_t *) DEV_REG_ADDR(IL_TERMINAL, devno);
	pcb_t *p;

	switch (reg->transm_status & DEV_TERM_STATUS) {
		case 1: // Ready
			break;
		case 5: // Character transmitted
			p = (pcb_t *) headBlocked(&semaphores.dev_term_trs[devno], 0);
			if (p) {
				p->p_s.a1 = reg->transm_status;
				sys3_SEMOP((int) &semaphores.dev_term_trs[devno], 1, 0);
				reg->transm_command = DEV_C_ACK;
			} else {
				fatal("Handle terminal trasm error\n");
			}
			break;
		default:
			fatal("Unknown terminal status");
	}

	switch (reg->recv_status & DEV_TERM_STATUS) {
		case 1: // Ready
			break;
		case 5: // Character received
			p = (pcb_t *) headBlocked(&semaphores.dev_term_rcv[devno], 0);
			if (p) {
				p->p_s.a1 = reg->recv_status;
				sys3_SEMOP((int) &semaphores.dev_term_rcv[devno], 1, 0);
				reg->recv_command = DEV_C_ACK;
			}
			break;
		default:
			fatal("Unknown terminal status");
	}
}

void handleDevice(int device, int dev_sem[]) {
	memaddr *line = (memaddr *) CDEV_BITMAP_ADDR(device);
	int devno = firstActiveDevice(*line);
	dtpreg_t *reg = (dtpreg_t *) DEV_REG_ADDR(device, devno);
	pcb_t *p;

	p = (pcb_t *) headBlocked(&dev_sem[devno], 0);
	if (!p) {
		fatal("No devices to receive interrupt\n");
	}
	p->p_s.a1 = reg->data1 << 8 | reg->status;
	reg->command = DEV_C_ACK;
	sys3_SEMOP((int) &dev_sem[devno], 1, 0);
}
