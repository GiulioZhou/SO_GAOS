#ifndef _UTIL_H
#define _UTIL_H

#include "types.h"

#define BIT(x) (1<<x)

// Prints a number in hex
void printHex(size_t);

void tprint(char *);

void *memset(void *s, int c, size_t n);
void *memcpy(void *dst, void *src, size_t n);
void dprint(char *s);
uint min(uint a, uint b);
int isNull(state_t *state);
int memcmp(const void* s1, const void* s2,size_t n);

#endif
