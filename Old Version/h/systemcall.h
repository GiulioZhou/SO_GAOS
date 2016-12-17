
#include "types.h"
extern pid_t newPid(pcb_t *proc);


pid_t createProcess(state_t *statep);
void terminateProcess(pid_t pid);
void semaphoreOperation (int *semaddr, int weight);
void specifySysBpHandler(memaddr pc, memaddr sp, unsigned int flags);
void specifyTLBHandler(memaddr pc, memaddr sp, unsigned int flags);
void specifyPgmTrapHandler(memaddr pc, memaddr sp, unsigned int flags);
void exitTrap(unsigned int excptype, unsigned int retval);
void getCpuTime(cputime_t *global, cputime_t *user);
void waitClock();
unsigned int ioDevOp(unsigned int command, int intlNo, unsigned int dnum);
pid_t getPid();

