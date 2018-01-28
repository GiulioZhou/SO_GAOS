#include "passups.h"

extern pcb_t *current_process;

// ======================== Exception Handlers ============================== //

// Handle trap exceptions
void handle_exc_trap(int cause, memaddr bad_addr) {
  SYSCALL(TERMINATE, 0, 0, 0);
  fatal("Trap handler error: the process hasn't been terminated\n");
}

// Handle tlb exceptions
void handle_exc_tlb(int cause, memaddr bad_addr) {
  int res = 0;
  switch (cause) {
  case TLBLEXCEPTION:
  case TLBSEXCEPTION:
  case UTLBLEXCEPTION:
  case UTLBSEXCEPTION:
    pager(bad_addr);
    break;
  case EXC_ADDRINVLOAD:
    usr21_TERMINATE();
    break;
  case EXC_ADDRINVSTORE:
    usr21_TERMINATE();
    break;
  default:
    //REVIEW lasciamo queste stampe?
    tprint("TLB exception with cause: ");
    tprintHex(cause);
    tprint("offending address: ");
    tprintHex(bad_addr);
    usr21_TERMINATE();
  }
  SYSCALL(EXITTRAP, PCB_FLAG_TLB_5, res, 0);
}

// Handle syscall exceptions
int exc_syscall(state_t *s) {
  switch (s->a1) {
  case 12:
    return usr12_READTERMINAL(s->a2);
  case 13:
    return usr13_WRITE_TO_TERMINAL(s->a2, s->a3);
  case 14:
    usr14_VSEMVIRT(s->a2, s->a3);
    return 0;
  case 15:
    usr15_PSEMVIRT(s->a2, s->a3);
    return 0;
  case 16:
    return usr16_DELAY(s->a2);
  case 17:
    return usr17_DISKPUT(s->a2, s->a3, s->a4);
  case 18:
    return usr18_DISKGET(s->a2, s->a3, s->a4);
  case 19:
    return usr19_WRITEPRINTER(s->a2, s->a3);
  case 20:
    return usr20_GETTOD();
  case 21:
    return usr21_TERMINATE();
  }
  fatal("Unknown usrcall\n");
}

// Handle both syscall/breakpoint exceptions
void handle_exc_sysbp(int cause) {
  int res;
  // Get the old process state
  state_t *s = &current_process->p_excpvec[EXCP_SYS_OLD];

  switch (cause) {
  case EXC_SYSCALL:
    res = exc_syscall(s);
    break;
  default:
    disableInterrupts();
    fatal("SYS/BP handler error: unknown exception cause\n");
  }
  SYSCALL(EXITTRAP, PCB_FLAG_SYS_4, res, 0);
  fatal("Exit trap failed\n");
}
