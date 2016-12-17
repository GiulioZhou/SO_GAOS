#include "const.h"
#include "types.h"


#define UPROCMAX 8 //non siamo sicuri del'8 però


typedef struct {
    uint entry_hi;
    uint entry_lo;
} ptentry_t;

typedef struct {
    uint header;
    ptentry_t entries[MAX_KPAGES];
} kptbl_t;


typedef struct {
    uint header;
    ptentry_t entries[MAX_PAGES];
} uptbl_t;

typedef struct {
    kptbl_t *kseg0;
    uptbl_t *useg2;
    uptbl_t *useg3;
} segtbl_entry_t;


/*---strutture per paginazione----*/

segtbl_entry_t *seg_table = (segtbl_entry_t *) SEGTABLE_START;

kptbl_t kseg0_ptbl;
uptbl_t useg2_ptbl[MAXPROC];
uptbl_t useg3_ptbl;



/*---- semafori ----*/

int uproc_semaphore[UPROCMAX];

/*---- state_t per processi vari --- */
state_t delay_state;




void initPageTable(uint *header, ptentry_t entries[], int num, int segno, int flags_lo) {
    *header = (PAGE_TBL_MAGICNO << 24) | num;
    for (int i = 0; i < num; i++){
		/* set up OS pages */
        // REVIEW: forse va messo questo (0x20000 + i) al posto di solo i, però boh
		entries[i].entry_hi = (i << 12) | (segno << 30);
		entries[i].entry_lo =  i << 12 | flags_lo;
	}
}

void initProcess() {
    uint asid = ...?
    pcb_t *p;
    seg_table[asid].kseg0 = &kseg0_ptable;
    seg_table[asid].useg2 = &p->p_ptable;
    seg_table[asid].useg3 = &useg3_ptable;
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
}

int test() {
    initPageTable(&kseg0_ptbl.header, kseg0_ptbl.entries, MAX_KPAGES, KSEGO,
            ENTRYLO_DIRTY | ENTRYLO_VALID | ENTRYLO_GLOBAL);
    initPageTable(&useg3_ptbl.header, useg3_ptbl.entries, MAX_PAGES, USEG3,
            ENTRYLO_GLOBAL);

    for (int j=0, j < UPROCMAX, j++) {
      uproc_semaphore[j]=0;
    }

    initADL();

    STST(&delay_state);

    delay_state.sp = delay_state.sp - 1024; //bisogna fare una const per la dim di una pag
    delay_state.pc = (memaddr)delay_deamon;
    delay_state.cpsr = STATUS_ALL_INT_ENABLE(delay_state.cpsr);
    delay_state->p_s.cpsr = STATUS_SYS_MODE;
    delay_state->p_s.cpsr = STATUS_ENABLE_TIMER(delay_state->p_s.cpsr);

    // System Control Register
    delay_state->CP15 = ??;


}
