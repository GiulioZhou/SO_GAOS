#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <libuarm.h>
#include <uARMconst.h>
#include "types.h"
#include "util.h"
#include "asl.h"
#include "pcb.h"
#include "scheduler.h"

extern pcb_t *current_process;
extern int s_pseudo_clock;
extern int sem_term_trs[];
extern int sem_term_rcv[];

// Syscalls list
void sys1_CREATEPROCESS(int a2, int a3, int a4);
void sys2_TERMINATEPROCESS(int a2, int a3, int a4);
void sys3_SEMOP(int a2, int a3, int a4);

void sys4_SPECSYSHDL(int a2, int a3, int a4);
void sys5_SPECTLBHDL(int a2, int a3, int a4);
void sys6_SPECPGMTHDL(int a2, int a3, int a4);
void sys7_EXITTRAP(int a2, int a3, int a4);

void sys8_GETCPUTIME(int a2, int a3, int a4);
void sys9_WAITCLOCK(int a2, int a3, int a4);
void sys10_IODEVOP(int a2, int a3, int a4);
void sys11_GETPID(int a2, int a3, int a4);
void sys12_READ_FROM_TERMINAL(int a2, int a3, int a4);
void sys13_WRITE_FROM_TERMINAL(int a2, int a3, int a4);

#endif
