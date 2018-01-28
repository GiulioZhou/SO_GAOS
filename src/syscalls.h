#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <libuarm.h>
#include <uARMconst.h>
#include "types.h"
#include "util.h"
#include "asl.h"
#include "pcb.h"
#include "scheduler.h"
#include "dma.h"
#include "swap.h"


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


// Usercalls list
int usr12_READTERMINAL(int a2);
int usr13_WRITE_TO_TERMINAL(int a2, int a3);
void usr14_VSEMVIRT (int *semaddr, int weight);
void usr15_PSEMVIRT (int *semaddr, int weight);
int usr16_DELAY(int a2);
int usr17_DISKPUT(int a2, int a3, int a4);
int usr18_DISKGET(int a2, int a3, int a4);
int usr19_WRITEPRINTER(int virtAddr, int len);
int usr20_GETTOD();
int usr21_TERMINATE();

#endif
