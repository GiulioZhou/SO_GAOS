#ifndef _CONST_H_
#define _CONST_H_

#include <uARMconst.h>
#include <arch.h>

// Some useful const
#define PRINTCHR	2
#define BYTELEN	8
#define RECVD	5
#define TRANSM 5

#define STATMASK      0xFF
#define TERMSTATMASK	0xFF
#define TERMCHARMASK	0xFF00

#define TERMREAD      0x80000000
#define TERMWRITE     0

#define NEW_LINE      0xA

#define SECOND		100000

// Max active processes
#define MAXPROC 20

// The swap disk must have at least :
#define MAX_HEAD 2
#define MAX_SECT 8

// Max user processes
#define UPROCMAX 8

// Max pages for each process
#define MAX_PAGES 32
#define MAX_KPAGES 84

// Swap pool size
#define SWAP_POOL_SIZE (UPROCMAX*2)

//ASID&PID delay delay_deamon
#define D_ASID 1
#define D_PID 3

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

// (phase3)-handled USERCALL values
#define READTERMINAL     12
#define WRITETERMINAL    13
#define VSEMVIRT         14
#define PSEMVIRT         15
#define DELAY            16
#define DISK_PUT         17
#define DISK_GET         18
#define WRITEPRINTER     19
#define GETTOD           20
#define TERMINATE        21

// pcb exception states vector constants
#define EXCP_SYS_OLD  0
#define EXCP_TLB_OLD  1
#define EXCP_PGMT_OLD 2
#define EXCP_SYS_NEW  3
#define EXCP_TLB_NEW  4
#define EXCP_PGMT_NEW 5
#define EXCP_COUNT    6

// PCB flags
#define PCB_FLAG_SYS_4 1
#define PCB_FLAG_TLB_5 2
#define PCB_FLAG_PGT_6 4

#define EXCP_COUNT 6

// device types count with separate terminal read and write devs
#define N_DEV_TYPES (N_EXT_IL+1)

// backing store device diskNo
#define DISK0 0

// number of disks
#define MAX_DISK_INDEX 7

// kseg0 structure
#define OS_CODE_BASE 0
#define DISKDMA_BUF_BASE (MAX_PAGES-1)
#define EXEC_HAND_BASE (DISKDMA_BUF_BASE + 8) //stacks for exception handlers


#define ADDR_OS 0x8000
#define ADDR_DMA_BUF     (ADDR_OS          + FRAME_SIZE*32         ) //0x28000
#define ADDR_UPROC_EXC   (ADDR_DMA_BUF     + FRAME_SIZE*UPROCMAX   ) //0x30000
#define ADDR_DELAY_STACK (ADDR_UPROC_EXC   + FRAME_SIZE*UPROCMAX*2 ) //0x40000
#define ADDR_DELAY_EXC   (ADDR_DELAY_STACK + FRAME_SIZE            ) //0x41000
#define ADDR_PAGE_POOL   (ADDR_DELAY_EXC   + FRAME_SIZE*2          ) //0x43000

#define DELAY_STACK (ADDR_DELAY_EXC/FRAME_SIZE)


#define LAST_WORD (FRAME_SIZE-WORD_SIZE)
#endif
