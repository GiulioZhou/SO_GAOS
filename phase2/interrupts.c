#include <pcb.h>
#include <asl.h>
#include <initial.h>
#include <scheduler.h>
#include <exceptions.h>
#include <interrupts.h>

#include <libuarm.h>
#include <arch.h>

typedef unsigned char byte;

byte firstDevice(byte bitmap) {
	unsigned int n;
	for (n = 0; n < 8; n++, bitmap/= 2) {
		if (bitmap & 1) return n;
	}
	return -1;
}

cputime_t interStart; //mi sa che devo farla globale

int device_numb(memaddr *pending){
	int bitmap= *pending;
	int devn;
	int i;
	for (i=0; i<8; i++){ //itero per tutti e 8 i bit
		if (1 & bitmap){ //se l'iesimo bit era acceso
			devn=i; // lo salvo e lo ritorno
			break;
		}
		else{ //altrimenti shifto
			bitmap >> 1;
		}
	}
	if(i>8){//non c'è stato un break che ci ha fatti uscire dal ciclo
		PANIC(); //non sapevo cosa mettere di errore e ho messo PANIC
	}
	return devn;//vorrei un caso di errore ma non so bene come farlo
}


void intHandler(){

	// state_t *retState;
	int cause;

	interStart = getTODLO();	//salvo il tempo in cui cominciamo la gestione
/*
	Decremento pc all'istruzione che stavamo eseguendo  --> riguardare
	retState = (state_t *) INT_OLDAREA;
	retState->pc = retState->pc - 4;
*/
	cause = getCAUSE();		//salvo la causa dell'interrupt

	if(currentProcess != NULL){
		currentProcess->p_userTime += interStart - userTimeStart;
		currentProcess->p_CPUTime += interStart - CPUTimeStart;
		//copyState( retState, &currentProcess->p_s );
		copyState(&currentProcess->p_s, (state_t*)INT_OLDAREA);
		currentProcess->p_s.pc -= 4;
		
	}

	//gestisci in base alla causa
	if (CAUSE_IP_GET(cause, IL_TIMER)){	//Pseudoclock o time-slice
		tprint("fine timer\n");
		intTimer();
	}
	else if (CAUSE_IP_GET(cause, IL_DISK)){
		tprint("disk\n");
		intDev(IL_DISK);
	}
	else if (CAUSE_IP_GET(cause, IL_TAPE)){
		intDev(IL_TAPE);
		tprint("tape\n");
	}
	else if (CAUSE_IP_GET(cause, IL_ETHERNET)){
		intDev(IL_ETHERNET);
				tprint("eth\n");
	}
	else if (CAUSE_IP_GET(cause, IL_PRINTER)){
		intDev(IL_PRINTER);
		tprint("print\n");
	}
	else if (CAUSE_IP_GET(cause, IL_TERMINAL)){
		intTerm();
		tprint("terminal\n");
	}

	scheduler();
}


void intDev(int int_no){ //gestore dell'interruptdi device, ho come argomento la linea di interrupt su cui arriva la cosa
	tprint("intDev\n");
	int devnumb;
	memaddr  *pending;
	int *sem; //semaforo su cui siamo bloccati
	pcb_t *unblck_proc; //processo appena sbloccato
	dtpreg_t *devReg; //registro del device
	
	pending= (memaddr *)CDEV_BITMAP_ADDR(int_no);//indirizzo della bitmap dove ci dice su quali device pendono gli interrupt
	devnumb= device_numb(pending);//prendiamo solo uno dei device su cui pendiamo
	sem=&devices[int_no-DEV_IL_START][devnumb];
	devReg= (dtpreg_t *) DEV_REG_ADDR(int_no, devnumb);
	//da qui in poi Sara fai attenzione a come uso i puntatori che come al solito non sono sicura
	devReg->command = DEV_C_ACK;	//passo l'acknowledgement (DEV_C_ACK sta in uARMconst)

	if (*sem < 1){
		unblck_proc = headBlocked(sem);
		semaphoreOperation(sem,1); //device starting interrupt line DEVINTBASE = 3 --> const.h
		if (unblck_proc!=NULL){

			unblck_proc->p_s.a1=devReg->status;
		}
	}

}

/*
void intTerm(int int_no) {
	memaddr *line = (memaddr *) CDEV_BITMAP_ADDR(IL_TERMINAL);
	int devno = firstDevice(*line);
	
	termreg_t *reg = (termreg_t *) DEV_REG_ADDR(IL_TERMINAL, devno);
	reg->transm_command = DEV_C_ACK;
	
	LDST(&currentProcess->p_s);
}
*/

void intTerm(){
	int devnumb;
	memaddr  *pending;
	int *sem; //semaforo su cui siamo bloccati
	pcb_t *unblck_proc; //processo appena sbloccato
	termreg_t *termReg; //registro del device
	
	pending= (memaddr *)CDEV_BITMAP_ADDR(IL_TERMINAL);//indirizzo della bitmap dove ci dice su quali device pendono gli interrupt
	devnumb= firstDevice(*pending); //prendiamo solo uno dei device su cui pendiamo
	//devnumb= device_numb(pending); //prendiamo solo uno dei device su cui pendiamo
	termReg=(termreg_t *)DEV_REG_ADDR(IL_TERMINAL, devnumb);

	
	
	//Scrivere ha la priorità sul leggere, quidni prima leggiamo :
	//????? magari scriviamo prima?
	if ((termReg->transm_status & DEV_TERM_STATUS)== DEV_TTRS_S_CHARTRSM){//le cose in maiuscolo sono in uARMconst

		sem=&devices[IL_TERMINAL-DEV_IL_START][devnumb];//se è trasmissione allora il semaforo è quello di trasmissione
		termReg->transm_command=DEV_C_ACK;//riconosco l'interrupt
	tprint("Ora vedo se mi devo bloccare\n");
		if (*sem < 0){
	tprint("mi devo bloccare\n");
			unblck_proc = headBlocked(sem);
			semaphoreOperation(sem,1);
			if (unblck_proc!=NULL){
				unblck_proc->p_s.a1=termReg->transm_status;
			}

		}

	}
	else if ((termReg->recv_status & DEV_TERM_STATUS) == DEV_TRCV_S_CHARRECV){
		sem=&devices[IL_TERMINAL-DEV_IL_START+1][devnumb];//se è di ricevere allora il semaforo è l'ultimo
		termReg->recv_command=DEV_C_ACK;

		if (*sem < 1){
			unblck_proc = headBlocked(sem);
			semaphoreOperation(sem,1); //device starting interrupt line DEVINTBASE = 3 --> const.h
			if (unblck_proc!=NULL){
				unblck_proc->p_s.a1=termReg->recv_status;
			}
		}
		LDST(&currentProcess->p_s);

	}
}

void intTimer(){
	tprint("intTimer\n");
	if (current_timer=TIME_SLICE){
		if (currentProcess!=NULL){
			insertProcQ(&readyQueue, currentProcess);
			//currentProcess->p_CPUTime += getTODLO() - interStart; // che senso ha???
			currentProcess=NULL;
		}
	}else if (current_timer=PSEUDO_CLOCK){
		while (&pseudoClock < 0){
			semaphoreOperation (&pseudoClock, 1); 
		}
	}
}
