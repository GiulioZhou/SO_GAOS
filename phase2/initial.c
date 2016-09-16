#include <pcb.h>
#include <asl.h>
#include <initial.h>
#include <scheduler.h>
#include <exceptions.h>
#include <interrupts.h>

#include <libuarm.h>
#include <arch.h>

/* definizione della struttura state_t preso dal manuale uARM
 typedef struct{
 unsigned int a1; //r0
 unsigned int a2; //r1
 unsigned int a3; //r2
 unsigned int a4; //r3
 unsigned int v1; //r4
 unsigned int v2; //r5
 unsigned int v3; //r6
 unsigned int v4; //r7
 unsigned int v5; //r8
 unsigned int v6; //r9
 unsigned int sl; //r10
 unsigned int fp; //r11
 unsigned int ip; //r12
 unsigned int sp; //r13
 unsigned int lr; //r14
 unsigned int pc; //r15
 unsigned int cpsr;
 unsigned int CP15_Control;
 unsigned int CP15_EntryHi;
 unsigned int CP15_Cause;
 unsigned int TOD_Hi;
 unsigned int TOD_Low;
 } state_t;
 */
#define HEX_DIGITS 8

extern void testfun();


//Stampa un numero esadecimale
inline void printHex(size_t n) {
	char str[3+HEX_DIGITS];
	int i;
	
	str[0] = '0';
	str[1] = 'x';
	str[2+HEX_DIGITS] = 0;
	
	for (i = 2; i < 10; i++)
		str[i] = '0';
	
	i = 1+HEX_DIGITS;
	while (n > 0 && i > 1) {
		str[i] = 48+n%16;
		str[i]+= str[i] < 58 ? 0 : 7;
		
		n/= 16;
		i--;
	}
	
	tprint(str);
	tprint("\n");
}

//stampa lo state t
void printState(state_t s) {
	tprint("----- STATE -----\n");
	tprint("IP    ");
	printHex(s.ip);
	tprint("PC    ");
	printHex(s.pc);
	tprint("SP    ");
	printHex(s.sp);
	tprint("LR    ");
	printHex(s.lr);
	tprint("cpsr    ");
	printHex(s.cpsr);
	tprint("CP15_Control    ");
	printHex(s.CP15_Control);
	tprint("--- END STATE ---\n");

}
/*
#define QPAGE			1024

void p2(){
	tprint("hello it's p2 here\n");
	SYSCALL (TERMINATEPROCESS, 0, 0, 0);
}
*/
/*
void testfun() {
	pid_t pid = SYSCALL(GETPID, 0, 0, 0);
	tprint("Started process ");
	printHex(pid);
	
	
	
	state_t p2state;
	STST(&p2state);
	p2state.sp = p2state.sp - QPAGE;
	
	p2state.pc = (memaddr)p2;
	
	p2state.cpsr = STATUS_ALL_INT_ENABLE(p2state.cpsr);
	
	SYSCALL(CREATEPROCESS, (int)&p2state, 0, 0);
	tprint("ciao");
	
	
	//SYSCALL (TERMINATEPROCESS, pid, 0, 0);
//	for (;;) tprint("xyz\n");
	WAIT();
}
 */




//variabili globali

clist readyQueue;
pcb_t *currentProcess;
unsigned int processCount;
unsigned int softBlockCount;
pcb_t *active_pcb[MAXPROC];
int devices[DEV_USED_INTS+1][DEV_PER_INT];	//semafori per i device
int pseudoClock;							//è un semaforo

//funzione che inizializza un'area con il gestore passato come parametro
void populate(memaddr area, memaddr handler){ //memaddr è un tipo di dato unsigned int definito in const.h
	
	//creo la nuova area
	state_t *newArea;
	newArea = (state_t *) area;
	STST(newArea);
	
	newArea->pc = handler;
	newArea->sp = RAM_TOP;
	newArea->cpsr =  STATUS_ALL_INT_DISABLE(newArea->cpsr) | STATUS_SYS_MODE; //E' GIUSTO
}

//copia lo stato da src a dest
void copyState(state_t *dest, state_t *src){
	dest->a1 = src->a1;
	dest->a2 = src->a2;
	dest->a3 = src->a3;
	dest->a4 = src->a4;
	dest->v1 = src->v1;
	dest->v2 = src->v2;
	dest->v3 = src->v3;
	dest->v4 = src->v4;
	dest->v5 = src->v5;
	dest->v6 = src->v6;
	dest->sl = src->sl;
	dest->fp = src->fp;
	dest->ip = src->ip;
	dest->sp = src->sp;
	dest->lr = src->lr;
	dest->pc = src->pc;
	dest->cpsr = src->cpsr;
	dest->CP15_Control = src->CP15_Control;
	dest->CP15_EntryHi = src->CP15_EntryHi;
	dest->CP15_Cause = src->CP15_Cause;
	dest->TOD_Hi = src->TOD_Hi;
	dest->TOD_Low = src->TOD_Low;
}

int main(){
	int i,j;
	pcb_t *first;
	
	populate(SYSBK_NEWAREA, (memaddr) sysBpHandler);
	populate(PGMTRAP_NEWAREA, (memaddr) pgmTrapHandler);
	populate(TLB_NEWAREA, (memaddr) tlbHandler);
	populate(INT_NEWAREA, (memaddr) intHandler);
	
	initPcbs();
	initASL();
	
	processCount = 0;
	softBlockCount = 0;
	readyQueue.next=NULL;
	currentProcess = NULL;
	
	for(i=0; i<MAXPROC; i++)	//Inizializzazione della tabella dei pcb utilizzati
		active_pcb[i] = NULL;
	
	for(i=0; i<=DEV_USED_INTS+1; i++){	//Numero dei device (+1 perchè il terminale ne vale 2)
		for (j=0; j<DEV_PER_INT; j++){	//Numero degli interrupt line
			devices[i][j]=0;
		}
	}
	pseudoClock = 0;
	
	first = allocPcb();
	
	newPid(first); //newPid
	
	first->p_s.cpsr = STATUS_ENABLE_INT(first->p_s.cpsr) | STATUS_ENABLE_TIMER(first->p_s.cpsr) | STATUS_SYS_MODE;
	first->p_s.sp = RAM_TOP - FRAME_SIZE;
	first->p_s.pc = (memaddr)testfun;	//Ricorda di modificare qui e sopra
	
	insertProcQ(&readyQueue, first);
	processCount++;
	
	
//	initScheduler();
	scheduler();
	return 0;
}
