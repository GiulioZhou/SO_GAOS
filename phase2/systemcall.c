#include <systemcall.h>
#include <asl.h>
#include <pcb.h>
#include <initial.h>
#include <scheduler.h>
#include <exceptions.h>

#include <libuarm.h>


//Create Process SYS1
pid_t createProcess(state_t *statep){
	pid_t pid;
	pcb_t *newp;
	
	if((newp = allocPcb()) == NULL)
		return -1;
	else {
		copyState(&newp->p_s, statep); //Inizializzazione dello state
		processCount++;
		pid=newPid(newp);	//pidCounter progressivo
		
		insertChild(currentProcess, newp);
		insertProcQ(&readyQueue, newp);
		
		return pid;	//il valore di ritorno verrà salvato nel registro giusto dall'handler
	}
}

//Terminate Process SYS2
void terminateProcess(pid_t pid){
	int i;
	pcb_t *pToKill;
	pcb_t *pChild;
	pcb_t *pSib;
	int isSuicide;
	pToKill = NULL;
	isSuicide = FALSE;
	
	if(currentProcess->p_pid != pid){ //suicide
		if(pid == 0){
			pid = currentProcess->p_pid;
			isSuicide = TRUE;
		}
	}
	else{
		isSuicide = TRUE;
	}
	
	
	/* if(currentProcess->p_pid == pid || pid == 0){ //preferisco in quest'altro modo ma fa un controllo in più >.>
		if(pid == 0){
	 pid = currentProcess->p_pid;
		}
		isSuicide = TRUE;
	 }*/
	
	pToKill = active_pcb[pid-1]; //Pcb da eliminare
	if(pToKill == NULL){
		
		PANIC();} //qui ritornava -1 ma visto che io non faccio ritornare mando in PANIC in caso di errore
	
	if(onSem(pToKill)){
		
		if(!onDev(pToKill)){	//bloccato su un device
			semaphoreOperation ((int*)pToKill->p_cursem, pToKill->p_resource); //libero risorse prenotate
			pToKill = outBlocked(pToKill);	//tolgo dalla lista dei processi bloccati sul semaforo
			if (pToKill == NULL)
			
			PANIC();
			
		}
		else{	//non bloccato su un device
			softBlockCount--;	//qui o nell'interrupt handler?
		}
	}
	
	while(emptyChild(pToKill) == FALSE){	//elimino eventuali figli
		pChild = removeChild(pToKill);
		terminateProcess(pChild->p_pid);	//come vedere se non ci sono stati errori?
	}
	
	if(pToKill->p_parent!=NULL){
		if((pToKill = outChild(pToKill)) == NULL){ // scolleghiamo il processo dal suo genitore
			
			PANIC();
		}
	}
	
	active_pcb[pid-1] = NULL;
	freePcb(pToKill);
	
	
	
	if(isSuicide == TRUE)
		currentProcess = NULL;
	
	processCount--;
	
}

//Semaphore Operation SYS3
void semaphoreOperation (int *semaddr, int weight){
	if (weight==0){
		terminateProcess(0);	//oppure SYSCALL(TERMINATEPROCESS, SYSCALL(GETPID));
		
	}
	else{
		(*semaddr) += weight;  //!!!!!!!!! KERNEL PANIC non sempre dipende da chi chiama semop
		if(weight > 0){ //abbiamo liberato risorse
			
			pcb_t *p;
			p=headBlocked(semaddr);
			if(p!=NULL){
				if (p->p_resource > weight){
					p->p_resource = p->p_resource - weight;
				}
				else{
					p = removeBlocked(semaddr);
					if (p != NULL){
						p->p_resource=0;
						insertProcQ(&readyQueue, p);
					}
					else {PANIC();} //si attiva qaudno la removeBlocked fallisce
				}
			}
		}
		else{ // abbiamo allocato risorse
			if  (*semaddr < 0)  {
				if(insertBlocked(semaddr, currentProcess)){
					PANIC();
				}
				
				currentProcess->p_resource=weight;
				currentProcess->p_CPUTime += getTODLO() - CPUTimeStart;
				currentProcess = NULL;
			}
		}
	}
}

//Specify Sys/BP Handler SYS4
void specifySysBpHandler(memaddr pc, memaddr sp, unsigned int flags){
	
	if (isNull(&currentProcess->p_excpvec[EXCP_SYS_NEW]) ){
		state_t *sysBp_new = (state_t *) SYSBK_NEWAREA;
		sysBp_new->pc=pc;
		sysBp_new->sp=sp;
		sysBp_new->cpsr= sysBp_new->cpsr | flags; //non penso vada bene
		
		//questo dovrebbe copiare l'asid del currentProcess in quello della newArea. le macro che ho usato sono sempre in uARMconst
		sysBp_new->CP15_EntryHi=ENTRYHI_ASID_SET( sysBp_new->CP15_EntryHi, ENTRYHI_ASID_GET(currentProcess->p_s.CP15_EntryHi));
		
		copyState(&currentProcess->p_excpvec[EXCP_SYS_NEW], sysBp_new);
		
	}else{
		terminateProcess(0);	//caso in cui il vettore delle eccezioni è già stato settato -> chiamato più di una volta
	}
}


//Specify TLB Handler SYS5
void specifyTLBHandler(memaddr pc, memaddr sp, unsigned int flags){

	if (isNull(&currentProcess->p_excpvec[EXCP_SYS_NEW])){
		state_t *TLB_new = (state_t *) TLB_NEWAREA;
		TLB_new->pc=pc;
		TLB_new->sp=sp;
		TLB_new->cpsr=TLB_new->cpsr | flags;
		TLB_new->CP15_EntryHi=ENTRYHI_ASID_SET( TLB_new->CP15_EntryHi, ENTRYHI_ASID_GET(currentProcess->p_s.CP15_EntryHi));
		
		copyState(&currentProcess->p_excpvec[EXCP_TLB_NEW], TLB_new);
		
	}else{
		terminateProcess(0);
	}
}

//Specify Program Trap Handler (SYS6)
void specifyPgmTrapHandler(memaddr pc, memaddr sp, unsigned int flags){

	if (isNull(&currentProcess->p_excpvec[EXCP_SYS_NEW])){
		state_t *pgmTrap_new = (state_t *) PGMTRAP_NEWAREA;
		pgmTrap_new->pc=pc;
		pgmTrap_new->sp=sp;
		pgmTrap_new->cpsr=pgmTrap_new->cpsr | flags;
		pgmTrap_new->CP15_EntryHi=ENTRYHI_ASID_SET( pgmTrap_new->CP15_EntryHi, ENTRYHI_ASID_GET(currentProcess->p_s.CP15_EntryHi));
		
		copyState(&currentProcess->p_excpvec[EXCP_PGMT_NEW], pgmTrap_new);
		
	}else{
		terminateProcess(0);
	}
}


// Exit From Trap SYS7
void exitTrap(unsigned int excptype, unsigned int retval){
	
	//state_t *load;
	state_t *old_area = &currentProcess->p_excpvec[excptype];
	//metto il valore di ritorno nel registro a1
	old_area->a1=retval;
	//load=old_area;
	//posso fare così o devo usare la copy? qui metto anche la copy da decommentare in caso quella sopra non funzioni
	//
	//copyState(load, old_area);
	//
	
	//LDST(load);
	LDST(old_area);
}

//Get CPU Time SYS8
void getCpuTime(cputime_t *global, cputime_t *user){
	cputime_t current_cpu= currentProcess->p_CPUTime;
	cputime_t current_usr= currentProcess->p_userTime;
	
	current_cpu += getTODLO() - CPUTimeStart;	//aggiungo il tempo dal processo fino a questo punto (che dovrebbe essere in kernel mode)
	
	*global=current_cpu;
	*user=current_usr;
}


//Wait For Clock SYS9
void waitClock(){
	//blocco il processo e aumento il conto dei processi che aspettano interrupts
	softBlockCount++;
	semaphoreOperation(&pseudoClock, -1);
}

//I/O Device Operation (SYS10)
unsigned int ioDevOp(unsigned int command, int intlNo, unsigned int dnum){
	
	int dev; //perchè ci serve?
	dtpreg_t *devReg; //registri dei device normali
	termreg_t *termReg; //registro del terminale
	int is_read;
	
	if((intlNo >= 0)&&(intlNo < INT_TERMINAL)){//caso accesso a device tranne scrittura su terminale
		dev = intlNo - DEV_IL_START; //in arch.h: DEV_IL_START (N_INTERRUPT_LINES - N_EXT_IL) --> 8-5 = 3
		is_read = FALSE;
	}
	else{
		if(dnum & 0x10000000){// caso scrittura su terminale
			dev = intlNo - DEV_IL_START+1;
			dnum = dnum & 0x00000111;
			
			is_read = TRUE;
		}
		else{
			dev = intlNo - DEV_IL_START;
			is_read = FALSE;
			
		}
	}
	
	
	if (intlNo == IL_TERMINAL){//azioni su terminali
		termReg=(termreg_t *)DEV_REG_ADDR(intlNo, dnum);
		if (is_read){
			termReg->recv_command=command;
			semaphoreOperation(&devices[dev][dnum], -1);

			return termReg->recv_status;
		}
		else{
			termReg->transm_command=command;
			
			//succedono cose strane....
			semaphoreOperation(&devices[dev][dnum], -1);
			
			return termReg->transm_status;
		}
		
	}
	else{//azioni su altri device
		devReg=(dtpreg_t *)DEV_REG_ADDR(intlNo, dnum);
		devReg->command=command;
		semaphoreOperation(&devices[dev][dnum], -1);

		return devReg->status;
	}
	
}


// Get Process ID SYS11

pid_t getPid(){
	return currentProcess->p_pid;
}

//Che palle... questi due fanno più o meno le stesse cose, cambia quale system call ha chiamato l'errore, per il primo SYS5 e il secondo SYS6

void pgmTrapHandler(){
	
	currentProcess->p_userTime += getTODLO() - userTimeStart;
	state_t *pgmTrap_old = (state_t *) PGMTRAP_OLDAREA ;
	
	//Ci serve la causa e le specifiche dicono che la trovo in CP15_Cause.excCode, quindi per recuperare la causa uso la macro che segue (non sono sicura di come funziona la macro, quindi faccio come avevano fatto i tipi di cui mi fido nella sys/bp handler)
	unsigned int cause= getCAUSE(); //dice che manipola il Cause register delle eccezzioni, però non so
	cause= CAUSE_EXCCODE_GET(cause);
	
	/*-----------------Metodo alternativo-----------------
	 
	 unsigned int cause= pgmTrap_old->CP15_Cause;
	 cause=CAUSE_EXCCODE_GET(cause);
	 
	 */
	//Volgio fare questo -->if (currentProcess->p_excpvec[EXCP_PGMT_NEW]!=NULL)
	
	if (!isNull(&currentProcess->p_excpvec[EXCP_SYS_NEW])){//vuol dire che era stata chiamata la sys5
		state_t *proc_new_area = &currentProcess->p_excpvec[EXCP_PGMT_NEW];
		state_t *proc_old_area = &currentProcess->p_excpvec[EXCP_PGMT_OLD];
		// The processor state is moved from the PgmTrap Old Area into the processor state stored in the ProcBlk as the PgmTrap Old Area
		copyState(proc_old_area, pgmTrap_old);
		
		//and Cause register is copied from the PgmTrap Old Area into the ProcBlk PgmTrap New Area’s a1 register.
		proc_new_area->a1=cause;
		
		//Finally, the processor state stored in the ProcBlk as the SYS/Bp New Area is made the current processor state.
		LDST(proc_new_area);
	}else{//non è stata fatta la sys5
		
		terminateProcess(0);
		
	}
	
}