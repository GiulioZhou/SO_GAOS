#define FALSE		0
#define TRUE		1
#define PAGE_SIZE 4096
#define SECOND		100000
#define STRMAX 5

/* VM/IO support level SYS calls (can be used by u-procs) */
#define READTERMINAL	12
#define WRITETERMINAL	13
#define VSEMVIRT	14
#define PSEMVIRT	15
#define DELAY		16
#define DISK_PUT	17
#define DISK_GET	18
#define WRITEPRINTER	19
#define GETTOD		20
#define TERMINATE	21

#define DEV_S_READY 1
/* Include additional device status codes if needed */

#define SEG0		0x00000000
#define SEG1		0x40000000
#define SEG2		0x80000000
#define SEG3		0xc0000000

#define NULL ((void *)0)

#define CR 0x0a   /* carriage return as returned by the terminal */
