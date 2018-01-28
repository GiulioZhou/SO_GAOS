#ifndef _TYPES_H
#define _TYPES_H

#include "clist.h"
#include <uARMtypes.h>
#include "const.h"

// size_t (kept compatibility with system libraries for p0test.c)

// byte
typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned int size_t;
typedef unsigned int pid_t;
typedef unsigned int cputime_t;
typedef unsigned int memaddr;

typedef struct {
	int pseudo_clock;
	int dev_disk[8];
	int dev_tape[8];
	int dev_network[8];
	int dev_printer[8];
	int dev_term_trs[8];
	int dev_term_rcv[8];
} devices_sem_t;


struct clist {
	struct clist *next;
};


// Semaphore structure
typedef struct semd_t {
	int *s_semAdd; /* pointer to the semaphore */
	struct clist s_link; /* ASL linked list */
	struct clist s_procq; /* blocked process queue */
} semd_t;

// Process control block structure
typedef struct pcb_t {
	struct pcb_t *p_parent; /* pointer to parent */
	struct semd_t *p_cursem; /* pointer to the semd_t on which the process is blocked */
	pid_t p_pid;
	state_t p_s; /* processor state */
	state_t p_excpvec[EXCP_COUNT]; /*exception states vector*/
	byte p_flags;
	struct clist p_jobs; /* all allocated pcbs */
	struct clist p_list; /* process list */
	struct clist p_children; /* children list entry point*/
	struct clist p_siblings; /* children list: links to the siblings */
	cputime_t p_usr_time;
	cputime_t p_sys_time;
	int p_slice_time;
	int p_resources_needed;
	uint invoked_sys4; /* flag for checking if process called a SYS4 */
	uint invoked_sys5; /* flag for checking if process called a SYS5 */
	uint invoked_sys6; /* flag for checking if process called a SYS6 */

	int p_asid;

} pcb_t;

typedef struct {
	uint entry_hi;
	uint entry_lo;
} ptentry_t;

typedef struct {
	uint header;
	ptentry_t entries[MAX_KPAGES]; //64 non funziona,
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

//record whether the frame is in use or not
typedef struct {
	byte asid;
	byte seg_no;
	int	vpn;
	ptentry_t *pte;
} swap_pool_t;

//Disk sector
typedef struct swap_list_t {
	byte head;
	byte sect;
	struct clist next;
} swap_list_t;

//Delay structure
typedef struct delayd_t {
	int d_wake_time;
	int asid;
	int d_sem_index;
	struct clist d_link;
} delayd_t;


#endif
