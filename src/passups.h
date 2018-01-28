#ifndef _PASSUPS_H
#define _PASSUPS_H

#include "const.h"
#include "types.h"
#include "clist.h"
#include "util.h"


void handle_exc_trap(int cause, memaddr bad_addr);
void handle_exc_tlb(int cause, memaddr bad_addr);
int exc_syscall(state_t *s);
void handle_exc_sysbp(int cause);

#endif
