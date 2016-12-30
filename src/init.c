#include "const.h"
#include "types.h"




/*---strutture per paginazione----*/

segtbl_entry_t *seg_table = (segtbl_entry_t *) SEGTABLE_START;

kptbl_t kseg0_ptbl;
uptbl_t useg2_ptbl[MAXPROC];
uptbl_t useg3_ptbl;

swapPool_t swapPool[SWAP_POOL_SIZE]; //UPROCMAX * 2


/*---- semafori ----*/
//delay facility, the virtual P & V facility, the swap-pool/Pager service and one for each device’s device registers
int uproc_semaphore[UPROCMAX];
int swap_semaphore;

/*---- state_t per processi vari --- */
state_t delay_state;




void initPageTable(uint *header, ptentry_t entries[], int num, int segno, int flags_lo) {
	int i;
	*header = (PAGE_TBL_MAGICNO << 24) | num;
    for (i = 0; i < num; i++){
		// set up OS pages
        // REVIEW: forse va messo questo (0x20000 + i) al posto di solo i, però boh
		entries[i].entry_hi = ((0x20000 + i) << 12) | (segno << 30);
		entries[i].entry_lo =  ((0x20000 + i) << 12) | flags_lo;
	}
}

//init process e delay demon spostati sotto perchè mi danno fastidio alla vista u.u

int test() {
	int j;
	
	//Initialize the single Kseg0 PTE
    initPageTable(&kseg0_ptbl.header, kseg0_ptbl.entries, MAX_KPAGES, KSEG0_PTB_SIZE,
            ENTRYLO_DIRTY | ENTRYLO_VALID | ENTRYLO_GLOBAL);
	
	//Initialize the single Useg3 PTE
    initPageTable(&useg3_ptbl.header, useg3_ptbl.entries, MAX_PAGES, USEG3_PTB_SIZE,
            ENTRYLO_GLOBAL);

	//Initilize all VM-I/O support level semaphores
	for (i=0; j < UPROCMAX; j++) {
      uproc_semaphore[j]=0;
    }
	
	//Initialize the swap-pool data structure(s)
	for (i = 0; i < SWAP_POOL_SIZE; i++){
		swapPool[i].asid = -1;
		swapPool[i].pte = NULL;
		swapPool[i].segNo = 0;
		swapPool[i].pageNo = 0;
	}

	
/*    initADL();

    STST(&delay_state);

    delay_state.sp = delay_state.sp - 1024; //bisogna fare una const per la dim di una pag
    delay_state.pc = (memaddr)delay_deamon;
    delay_state.cpsr = STATUS_ALL_INT_ENABLE(delay_state.cpsr);
    delay_state->p_s.cpsr = STATUS_SYS_MODE;
    delay_state->p_s.cpsr = STATUS_ENABLE_TIMER(delay_state->p_s.cpsr);

    // System Control Register
    delay_state->CP15 = ??;
*/
	HALT();
}


/*
 
 void initProcess() {
	uint asid = ???;
 pcb_t *p;
 seg_table[asid].kseg0 = &kseg0_ptbl;
 //seg_table[asid].useg2 = &p->p_ptbl;
 seg_table[asid].useg3 = &useg3_ptbl;
 }
 
 void delay_deamon(){//fatta manca solo da inserire la V
 while(TRUE){
 int time;
 delayd_t head;
 sys9_WAITCLOCK(0, 0, 0);
 time = getTODLO();
 while(head=del_head(adl)){
 if(head->d_wake_time <= time) {
 //bisogna fare la V nel private sem
 //controlla acid, cercalo tra i bloccati e sbloccalo
 del_remove(adl);
 }
 else break;
 }
 }
 }*/

