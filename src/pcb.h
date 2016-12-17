#ifndef _PCB_H
#define _PCB_H

#include "const.h"
#include "types.h"
#include "clist.h"
#include "util.h"

// Init pcbs
void initPcbs();
pcb_t *allocPcb();

pcb_t *findPcb(pid_t pid);
void freePcb(pcb_t *p);

// Process queues management
void insertProcQ(struct clist *q, pcb_t *p);
pcb_t *removeProcQ(struct clist *q);
pcb_t *outProcQ(struct clist *q, pcb_t *p);
pcb_t *headProcQ(struct clist *q);

// Process tree management
int emptyChild(pcb_t *p);
void insertChild(pcb_t *parent, pcb_t *p);
pcb_t *removeChild(pcb_t *p);
pcb_t *outChild(pcb_t *p);

#endif
