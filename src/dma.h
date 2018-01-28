#ifndef _DMA_H_
#define _DMA_H_

#include "const.h"
#include "types.h"

void initDMA();
memaddr allocDMABuff();
void freeBuff(memaddr addr);

#endif
