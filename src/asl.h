#ifndef _ASL_H
#define _ASL_H

#include "const.h"
#include "types.h"
#include "clist.h"
#include "pcb.h"

extern struct clist ready_queue;

// Asl management functions
void initASL(void);
void initAVSL(void);
pcb_t *headBlocked(int *semAdd, int virt);
pcb_t *outBlocked(pcb_t *p);
pcb_t *removeBlocked(int *semAdd, int virt);
int insertBlocked(int *semAdd, pcb_t *p, int virt);

#endif
