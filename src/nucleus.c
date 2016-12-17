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

extern void test();
extern void test_x();
extern void test_y();

void printState(state_t s) {
    dprint("----- STATE -----\n");
    dprint("IP    ");
    printHex(s.ip);
    dprint("PC    ");
    printHex(s.pc);
    dprint("SP    ");
    printHex(s.sp);
    dprint("LR    ");
    printHex(s.lr);
    dprint("cpsr    ");
    printHex(s.cpsr);
    dprint("CP15_Control    ");
    printHex(s.CP15_Control);
    dprint("--- END STATE ---\n");
    // dprint("spsr    ");
    // printHex(s.spsr);
}

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
    pcb->p_s.pc = (memaddr) test;
    pcb->p_s.sp = RAM_TOP-FRAMESIZE;

    return pcb;
}


void test_1() {
    uint pid = SYSCALL(GETPID, 0, 0, 0);

    int i;
    while(1) {
        for (i = 0; i < 6000; i++);
        SYSCALL(IODEVOP, 0x00006102, INT_TERMINAL, 0);
    }

    WAIT();
}
void test_2() {
    uint pid = SYSCALL(GETPID, 0, 0, 0);
        int i;
    while (1) {
        for (i = 0; i < 20000; i++);
        SYSCALL(IODEVOP, 0x00006202, INT_TERMINAL, 0);
    }
    WAIT();
}

int main() {
    initExceptionHandlers();
    initPcbs();
    initASL();


    memset(&semaphores, 0, sizeof(semaphores));

    // Create init and start the scheduler
    pcb_t *init;
    init = makeInit();
    init->p_pid = 1;
    init->p_s.pc = (memaddr) test;
    // insertProcQ(&ready_queue, init);
    // init = makeInit();
    // init->p_pid = 2;
    // init->p_s.pc = (memaddr) test_y;
    // init->p_s.sp = RAM_TOP - FRAME_SIZE*2;
    schedStart(init);
    return 0;
}
