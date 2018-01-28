#include "dma.h"

int dma_buff[UPROCMAX];

void initDMA() {
  memset(dma_buff, 0, sizeof(dma_buff));
}

memaddr allocDMABuff() {
    int i=0;
    if (i < UPROCMAX) {
      dma_buff[i] = 1;
      return ADDR_DMA_BUF+FRAME_SIZE*i;
    } else {
      fatal("All DMA buffers are occupied\n");
      PANIC();
    }
}


void freeBuff(memaddr addr) {
  int i = (addr-ADDR_DMA_BUF)/FRAME_SIZE;
  if (i<0 || i > UPROCMAX){
    fatal("DMA buffer index is \n");
    PANIC();
  }
  dma_buff[i] = 0;
}
