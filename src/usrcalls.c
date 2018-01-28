#include "syscalls.h"

extern pcb_t *current_process;
extern int uproc_semaphore[];
extern swap_pool_t swap_pool[SWAP_POOL_SIZE];
extern cputime_t boot_start;
extern int master_sem;
extern int swap_semaphore;
extern uint uprocess_c;


int usr12_READTERMINAL(int a2){
	static int read_sem = 1;
	char *virtAddr = (char *)a2;
	int status;
	char recv;
	int char_counter = 0;

	if ((memaddr) virtAddr < USEG2BASE) {
		usr21_TERMINATE();
	}

	SYSCALL(SEMOP, (int) &read_sem, -1, 0);

	if (current_process) {
		do {
			status = SYSCALL(IODEVOP, DEV_TRCV_C_RECVCHAR, INT_TERMINAL, TERMREAD);

			if ((status & TERMSTATMASK) != RECVD )
				return -(status & TERMSTATMASK);

			recv = (status & TERMCHARMASK) >> BYTELEN;
			*virtAddr = recv;
			virtAddr++;
			char_counter++;

		} while(recv != NEW_LINE);
	}

	SYSCALL(SEMOP, (int) &read_sem, 1, 0);
	return char_counter;
}

int usr13_WRITE_TO_TERMINAL(int a2, int a3) {
	static int write_sem = 1;
	int status;
	if ((memaddr) a2 < USEG2BASE || a3 < 0 || a3 > 128) {
		usr21_TERMINATE();
	}
	int lenght = a3;

	SYSCALL(SEMOP, (int) &write_sem, -1, 0);

	if (current_process) {
		char *cur_char = (char *) a2;
		while (a3) {

			int command = DEV_TTRS_C_TRSMCHAR | (((char)*cur_char) << BYTELEN);
			status = SYSCALL(IODEVOP, command, INT_TERMINAL, 0);
			if ((status & TERMSTATMASK) != TRANSM) {
				int error = -(status & TERMSTATMASK);
				return error;
			}
			if (((status & TERMCHARMASK) >> BYTELEN) != *cur_char) {
				fatal("Wrong character transmitted\n");
			}
			cur_char++;
			a3--;
		}
	}
	SYSCALL(SEMOP, (int) &write_sem, 1, 0);
	return lenght;
}

void usr14_VSEMVIRT (int *semaddr, int weight){
	if (((memaddr) semaddr) >> 30 != USEG3 || weight <= 0) {
		usr21_TERMINATE();
	}

	*semaddr+= weight;
	SYSCALL(SEMOP,(int) semaddr, weight, *semaddr);

}

void usr15_PSEMVIRT (int *semaddr, int weight){
	if (((memaddr) semaddr) >> 30 != USEG3 || weight <= 0) {
		usr21_TERMINATE();
	}
	*semaddr-= weight;
	SYSCALL(SEMOP, (int) semaddr, -weight, *semaddr);
}

int usr16_DELAY(int secCnt) {
	int asid = getASID();
	int index;
	int tod = getTODLO();
	if (secCnt < 0) {
		usr21_TERMINATE();
	}

	index = del_insert(asid, secCnt*SECOND);

	if (index == -1){
		fatal("All delay semaphores occupied\n");
	}

	SYSCALL(SEMOP,&uproc_semaphore[index],-1,0);
	return 0;
}

int usr17_DISKPUT(int addr, int disk_number, int disk_sector) {
	int cmd, res;
	int *device_sem;
	memaddr dma;

	if((disk_number == DISK0)||(addr <= USEG2BASE)||(disk_number > MAX_DISK_INDEX)){
		usr21_TERMINATE();
	}

	if(addr >> 30 == KSEG0){
		fatal("Cannot write in kseg0");
	}

	disableInterrupts();
	memaddr buff = allocDMABuff();
	memcpy(buff, addr, FRAME_SIZE);
	enableInterrupts();

	res= disk_io(buff, disk_number, disk_sector, DEV_DISK_C_WRITEBLK);

	freeBuff(buff);
	return res == DEV_S_READY ? res : -res;
}

int usr18_DISKGET(int addr, int disk_number, int disk_sector) {
	int cmd, res;
	dtpreg_t *dev;

	if((disk_number == DISK0)||(addr <= USEG2BASE)||(disk_number > MAX_DISK_INDEX)){
		usr21_TERMINATE();
	}

	if(addr >> 30 == KSEG0){
		fatal("Cannot read in kseg0");
	}

	memaddr buff = allocDMABuff();
	res= disk_io(buff, disk_number, disk_sector, DEV_DISK_C_READBLK);

	memcpy(addr,(void *) buff, FRAME_SIZE);
	freeBuff(buff);

	return res == DEV_S_READY ? res : -res;
}

int usr19_WRITEPRINTER(int virtAddr, int len){
	static int write_sem = 1;
	int status;
	int lenght = len;
	dtpreg_t *dev;

	if ((virtAddr < USEG2BASE) || (len < 0 || len > 128)){
		usr21_TERMINATE();
	}

	SYSCALL(SEMOP, (int) &write_sem, -1, 0);

	if (current_process) {
		char *cur_char = (char *) virtAddr;
		dev=(dtpreg_t *) DEV_REG_ADDR(INT_PRINTER, 0); //We use only one printer
		while (len) {
			dev->data0 = *cur_char;
			status = SYSCALL(IODEVOP, DEV_PRNT_C_PRINTCHR, INT_PRINTER, 0);

			if ((status & TERMCHARMASK) != DEV_S_READY) {
				return -status;
			}

			cur_char++;
			len--;
		}
	}

	return lenght;
}

int usr20_GETTOD(){
	return getTODLO()-boot_start;
}

int usr21_TERMINATE() {
	int i;
	int asid = getASID();

	// clean swap_pool
	SYSCALL(SEMOP, (int) &swap_semaphore, -1, 0);
	for (i = 0; i < SWAP_POOL_SIZE; i++) {
		if(swap_pool[i].asid = asid)
			resetFrame(i);
	}
	TLBCLR();
	SYSCALL(SEMOP, (int) &swap_semaphore, 1, 0);

	uprocess_c--;

	//All u-procs terminated
	if(uprocess_c == 0){
		SYSCALL(SEMOP, (uint)&master_sem, 1, 0);
	}
	SYSCALL(TERMINATEPROCESS, 0, 0, 0);
}
