#ifndef _UTIL_H
#define _UTIL_H

#include "types.h"

#define BIT(x) (1<<x)

void enableInterrupts();
void enableInterrupts();
int getASID();
void *memset(void *s, int c, size_t n);
void *memcpy(void *dst, void *src, size_t n);
uint min(uint a, uint b);
int isNull(state_t *state);
int memcmp(const void* s1, const void* s2,size_t n);

state_t setUpState(state_t state, int mode, int interrupt, int vm, int asid);
void tprintHex(size_t n);
void fatal(char *msg);

#endif
