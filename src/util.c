#include "util.h"


/*
 * Prints a number in its hexadecimal form, ex. 0x0012ACF1
 */
inline void printHex(size_t n) {
    int output_length = 8;
    char str[3+output_length];
    int i;

    str[0] = '0';
    str[1] = 'x';
    str[2+output_length] = 0;

    for (i = 2; i < 10; i++)
        str[i] = '0';

    i = 1+output_length;
    while (n > 0 && i > 1) {
        str[i] = 48+n%16;
        str[i]+= str[i] < 58 ? 0 : 7;

        n/= 16;
        i--;
    }

    dprint(str);
    dprint("\n");
}

/*
 * Set every byte from s to s+n to c.
 * Returns a pointer to s.
 */
void *memset(void *s, int c, size_t n)
{
    unsigned char* p=s;
    while(n--)
        *p++ = c;
    return s;
}

/*
 * Copy a block of bytes from src to dst
 */

void *memcpy(void *dst,void *src, size_t n)
{
    unsigned char* d=dst;
    unsigned char* s=src;
    while(n--)
        *d++ = *s++;
    return dst;
}

void dprint(char *s) {
   #ifdef DEBUG
       tprint(s);
   #endif
}

uint min(uint a, uint b) {
    return a < b ? a : b;
}


int memcmp(const void* s1, const void* s2,size_t n)
{
	const unsigned char *p1 = s1, *p2 = s2;
	while(n--)
		if( *p1 != *p2 )
			return *p1 - *p2;
		else
			p1++,p2++;
	return 0;
}


int isNull(state_t *state){
	state_t state_null;
	memset(&state_null, 0, sizeof(state_t));
	if (!memcmp(state,&state_null, sizeof(state_t))) return 1;
	return 0;
}
