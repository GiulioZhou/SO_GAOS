#ifndef EXCEP_H_INCLUDED
#define EXCEP_H_INCLUDED

#include "types.h"

extern pid_t newPid(pcb_t *proc);

void sysBpHandler();
void pgmTrapHandler();
void tlbHandler();
void bpHandler();



int memcmp(const void* s1, const void* s2,size_t n);
int isNull(state_t *state);

#endif
