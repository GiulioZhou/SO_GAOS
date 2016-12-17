#ifndef _CONST_H_
#define _CONST_H_

#include <uARMconst.h>
#include <arch.h>




// Max active processes
#define MAXPROC 20

// Max pages for each process
#define MAX_PAGES 32
#define MAX_KPAGES 50 //messo da noi perché le specifiche dicono che devono essere circa 50

// Scheduling constants
#define SCHED_TIME_SLICE 5000     // in microseconds, aka 5 milliseconds
#define SCHED_PSEUDO_CLOCK 100000UL // pseudo-clock tick "slice" length
#define SCHED_BOGUS_SLICE 5000000  // just to make sure


// nucleus (phase2)-handled SYSCALL values
#define CREATEPROCESS    1
#define TERMINATEPROCESS 2
#define SEMOP            3
#define SPECSYSHDL       4
#define SPECTLBHDL       5
#define SPECPGMTHDL      6
#define EXITTRAP         7
#define GETCPUTIME       8
#define WAITCLOCK        9
#define IODEVOP          10
#define GETPID           11

#define SYSCALL_MAX 11

// pcb exception states vector constants
#define EXCP_SYS_OLD  0
#define EXCP_TLB_OLD  1
#define EXCP_PGMT_OLD 2
#define EXCP_SYS_NEW  3
#define EXCP_TLB_NEW  4
#define EXCP_PGMT_NEW 5
#define EXCP_COUNT    6

// PCB flags
#define PCB_FLAG_SYS_4 0x01
#define PCB_FLAG_TLB_5 0x02
#define PCB_FLAG_PGT_6 0x04

#define EXCP_COUNT 6

// device types count with separate terminal read and write devs
#define N_DEV_TYPES (N_EXT_IL+1)


#endif
