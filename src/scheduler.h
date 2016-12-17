#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "types.h"
#include "const.h"
#include "pcb.h"
#include "syscalls.h"
#include <libuarm.h>

void schedStart(pcb_t *pcb);
void scheduler();

#endif
