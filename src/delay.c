#include "delay.h"

extern int uproc_semaphore[UPROCMAX];
extern kptbl_t kseg0_ptbl;
extern uptbl_t useg2_ptbl[UPROCMAX];
extern uptbl_t useg3_ptbl;
extern segtbl_entry_t *seg_table;

state_t delay_state;

// =============== Process Initialization and Bootstrapping ================= //
void delay_deamon(){
	int i;

	SYSCALL(SPECSYSHDL,  (memaddr)handle_exc_sysbp, (memaddr) ADDR_DELAY_EXC + LAST_WORD, STATUS_ALL_INT_ENABLE(STATUS_SYS_MODE));
	SYSCALL(SPECTLBHDL,  (memaddr)handle_exc_tlb,   (memaddr) ADDR_DELAY_EXC+FRAME_SIZE + LAST_WORD, STATUS_ALL_INT_ENABLE(STATUS_SYS_MODE));
	SYSCALL(SPECPGMTHDL, (memaddr)handle_exc_trap,  (memaddr) ADDR_DELAY_EXC + LAST_WORD, STATUS_ALL_INT_ENABLE(STATUS_SYS_MODE));

	while(1){
		SYSCALL(WAITCLOCK, 0, 0, 0);
		while( (i = del_remove()) != -1){
			SYSCALL(SEMOP,&uproc_semaphore[i],1,0);
		}
	}
}

int init_del_deamon(){

	seg_table[D_ASID].kseg0 = &kseg0_ptbl;
	delay_state = setUpState(delay_state, STATUS_SYS_MODE, 1, 1,D_ASID);
	delay_state.sp = ADDR_DELAY_STACK + FRAME_SIZE - WORD_SIZE;
	delay_state.pc = (memaddr)delay_deamon;

 SYSCALL(CREATEPROCESS, (int)&delay_state, D_ASID, 0);

}
