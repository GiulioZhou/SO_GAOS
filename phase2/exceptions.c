#include <asl.h>
#include <pcb.h>
#include <initial.h>
#include <scheduler.h>
#include <exceptions.h>

#include <libuarm.h>

//funzione per inizializzare il pid
pid_t newPid (pcb_t *proc){
	int i=0;
	while ( i < MAXPROC ) {
		if (active_pcb[i]==NULL){
			active_pcb[i]= proc;
			proc->p_pid=i+1;
			break;
		}
		i++;
	}
	if (i >= MAXPROC) //ci sono più pricessi di quanti ne posso avere
		PANIC();
	return i+1;
}

//vedere se è su semaforo
int onSem (pcb_t *pcb){
	return (pcb->p_cursem != NULL);
}

//???????????????????????????
//vedere se è bloccato sul device
int onDev (pcb_t *pcb){
	int i,j;
	if (onSem(pcb)){
		for (i=0; i<=DEV_USED_INTS+1; i++){
			for (j=0; j<DEV_PER_INT;j++){
				if (pcb->p_cursem->s_semAdd==&devices[i][j]){
					return TRUE;
				}
			}
		}
		
		if (pcb->p_cursem->s_semAdd==&pseudoClock) //non so se lo pseudoClock è considerato device
			return TRUE;
		//qui ci arriviamo se sono collegato a un semaforo ma non è device
		return FALSE;
	}
	
	//qui ci arriviamo se non era su un semaforo to begin with
	else return FALSE;
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

//per vedere se la struttura è nulla
int isNull(state_t *state){
	state_t state_null;
	memset(&state_null, 0, sizeof(state_t));
	if (!memcmp(state,&state_null, sizeof(state_t))) return 1;
	return 0;
}

void sysBpHandler(){
	
	state_t *sysBp_old = (state_t *) SYSBK_OLDAREA;
	copyState(&currentProcess->p_s, sysBp_old);
	
	int cause = CAUSE_EXCCODE_GET(currentProcess->p_s.CP15_Cause);		//Prendo la causa dell'eccezzione
	
	// Salva i parametri delle SYSCALL
	unsigned int sysc = sysBp_old->a1;
	unsigned int argv1 = sysBp_old->a2;
	unsigned int argv2 = sysBp_old->a3;
	unsigned int argv3 = sysBp_old->a4;
	
	if(cause == EXC_SYSCALL){ //caso system call
		//controlla di avere il permesso
		if((currentProcess->p_s.cpsr & STATUS_SYS_MODE) == STATUS_SYS_MODE){
			//sono in kernel mode quindi da qui faccio tutte le cose che devo fare
			//essendo sicura di essere in kernel mode posso dire che è finito il tempo utente del processo. Dovrei farlo appena entrata nell'handler? Non cred perché potrei essere capitata qui anche in user mode, solo qui ho la sicurezza di essere in kernel mode
			currentProcess->p_userTime += getTODLO() - userTimeStart;
			
			tprint("che sys call e'?\n");
			
			
			/* Gestisce ogni singola SYSCALL */
			switch(sysc)
			{
				case CREATEPROCESS:
					currentProcess->p_s.a1 = createProcess((state_t *) argv1); // se questo cast non funziona provare a fare argv1 di tipo U32
					break;
					
				case TERMINATEPROCESS:
					tprint("terminate process\n");
					terminateProcess((pid_t) argv1);
					
					break;
					
				case SEMOP:
					tprint("semop\n");
					semaphoreOperation((int *) argv1, (int) argv2);
					break;
					
				case SPECSYSHDL:
					specifySysBpHandler((memaddr) argv1, (memaddr) argv2, (unsigned int) argv3);
					break;
					
				case SPECTLBHDL:
					specifyTLBHandler((memaddr) argv1, (memaddr) argv2, (unsigned int) argv3);
					break;
					
				case SPECPGMTHDL:
					specifyPgmTrapHandler((memaddr) argv1, (memaddr) argv2, (unsigned int) argv3);
					break;
					
					
				case EXITTRAP:
					exitTrap((unsigned int) argv1, (unsigned int) argv2);
					break;
					
				case GETCPUTIME:
					getCpuTime((cputime_t *) argv1, (cputime_t *) argv2); //ancora non sono sicura che il cast sia giusto
					break;
					
				case WAITCLOCK:
					waitClock();
					break;
					
				case IODEVOP:
					tprint("iodevop\n");
					semaphoreOperation ((int *) argv1, (int) argv2);
					break;
					
				case GETPID:
					currentProcess->p_s.a1 = getPid();
					break;
					
					
					
				default:
					bpHandler();
			}
			
			if(currentProcess){
				LDST(&currentProcess->p_s);
			}
			else {				tprint("no currproc\n"); scheduler();}
			
		}
		else if ((currentProcess->p_s.cpsr & STATUS_USER_MODE) == STATUS_USER_MODE) {//qui nel caso sono in user mode e provo a fare una syscall faccio come mi dicono le specifiche, copiando le old aree giuste e alzando una trap chiamando l'handler delle trap
			tprint("User mode D:\n");
			if (sysc >= 1 && sysc <= SYSCALL_MAX){
				state_t *pgmTrap_old = (state_t *) PGMTRAP_OLDAREA;
				copyState(pgmTrap_old,sysBp_old);
				//CAUSE_EXCCODE_SET definito in uARMconst
				pgmTrap_old->CP15_Cause=CAUSE_EXCCODE_SET(pgmTrap_old->CP15_Cause, EXC_RESERVEDINSTR );
				pgmTrapHandler();
			}
		}
		
	}else if ( cause == EXC_BREAKPOINT ){ //caso breakpoint
		
		bpHandler();
		
		
	}else { //non è ne syscall ne breakpoint
		tprint("ne syscall ne breakpoint");
		PANIC();
	}
}

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
		
		return pid;	//il valore di ritorno verrà salvato nel registro giusto nell'handler
	}
}

//Terminate Process SYS2
void terminateProcess(pid_t pid){
	int i;
	pcb_t *pToKill;
	pcb_t *pChild;
	pcb_t *pSib;
	int isSuicide;
	tprint("1\n");
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
	tprint("2\n");
	printHex(isSuicide);
	
	/* if(currentProcess->p_pid == pid || pid == 0){ //preferisco in quest'altro modo ma fa un controllo in più >.>
		if(pid == 0){
	 pid = currentProcess->p_pid;
		}
		isSuicide = TRUE;
	 }*/
	
	pToKill = active_pcb[pid-1]; //Pcb da eliminare
	if(pToKill == NULL){
		tprint("3\n");
		
		PANIC();} //qui ritornava -1 ma visto che io non faccio ritornare mando in PANIC in caso di errore
	
	if(onSem(pToKill)){
		tprint("4\n");
		
		if(!onDev(pToKill)){	//bloccato su un device
			semaphoreOperation ((int*)pToKill->p_cursem, pToKill->p_resource); //libero risorse prenotate
			pToKill = outBlocked(pToKill);	//tolgo dalla lista dei processi bloccati sul semaforo
			tprint("5\n");
			if (pToKill == NULL)
				tprint("6\n");
			
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
			tprint("7\n");
			
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
	tprint("semOperation\n");
	if (weight==0){
		tprint("weight 0\n");
		terminateProcess(0);	//oppure SYSCALL(TERMINATEPROCESS, SYSCALL(GETPID));
		
	}
	else{
		tprint("wight diverso da 0\n");
		(*semaddr) += weight;  //!!!!!!!!! KERNEL PANIC non sempre dipende da chi chiama semop
		if(weight > 0){ //abbiamo liberato risorse
			tprint("libero risorse\n");
			
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
					else PANIC(); //si attiva qaudno la removeBlocked fallisce
				}
			}
		}
		else{ // abbiamo allocato risorse
			tprint("Ci servono risorse\n");
			if  (*semaddr < 0)  {
				if(insertBlocked(semaddr, currentProcess)){
					tprint("panic non insertblock");
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
		sysBp_new->cpsr=flags; //non penso vada bene
		
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
		TLB_new->cpsr=flags;
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
		pgmTrap_new->cpsr=flags;
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
void ioDevOp(unsigned int command, int intlNo, unsigned int dnum){
	
	int dev; //perchè ci serve?
	dtpreg_t *devReg; //registri dei device normali
	termreg_t *termReg; //registro del terminale
	int is_read;
	
	if((dnum >= 0)&&(dnum < N_DEV_PER_IL)){//caso accesso a device tranne scrittura su terminale
		dev = intlNo - DEV_IL_START; //in arch.h: DEV_IL_START (N_INTERRUPT_LINES - N_EXT_IL) --> 8-5 = 3
		
		is_read = FALSE;
	}
	else{
		if(dnum & 0x10000000){// caso scrittura su terminale
			dev = N_EXT_IL;
			dnum = dnum & 0x00000111;
			
			is_read = TRUE;
		}
		else{ //abbiamo chiamato la sys10 con un dnum che non esiste
			PANIC();
		}
	}
	
	
	if (intlNo == IL_TERMINAL){//azioni su terminali
		termReg=(termreg_t *)DEV_REG_ADDR(intlNo, dnum);
		if (is_read){
			termReg->recv_command=command;
		}
		else{
			termReg->transm_status=command;
		}
		
	}
	else{//azioni su altri device
		devReg=(dtpreg_t *)DEV_REG_ADDR(intlNo, dnum);
		devReg->command=command;
	}
	
	//basta quello che ho fatto sopra per "performare" il comando?
	
	//la cosa seguente in teoria dovrebbe bloccare il currentProcess nel semaforo del device giusto ma boh
	semaphoreOperation(&devices[dev][dnum], -1);
	
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

void tlbHandler(){
	currentProcess->p_userTime += getTODLO() - userTimeStart;
	state_t *tlb_old = (state_t *) TLB_OLDAREA ;
	
	//Ci serve la causa e le specifiche dicono che la trovo in CP15_Cause.excCode, quindi per recuperare la causa uso la macro che segue (non sono sicura di come funziona la macro, quindi faccio come avevano fatto i tipi di cui mi fido nella sys/bp handler)
	unsigned int cause= getCAUSE(); //dice che manipola il Cause register delle eccezzioni, però non so
	cause= CAUSE_EXCCODE_GET(cause);
	
	/*-----------------Metodo alternativo-----------------
	 
	 unsigned int cause= tlb_old->CP15_Cause;
	 cause=CAUSE_EXCCODE_GET(cause);
	 
	 */
	
	
	if (!isNull(&currentProcess->p_excpvec[EXCP_SYS_NEW])){//vuol dire che era stata chiamata la sys5
		state_t *proc_new_area = &currentProcess->p_excpvec[EXCP_TLB_NEW];
		state_t *proc_old_area = &currentProcess->p_excpvec[EXCP_TLB_OLD];
		// The processor state is moved from the tlb Old Area into the processor state stored in the ProcBlk as the TLB Old Area
		copyState(proc_old_area, tlb_old);
		
		//and Cause register is copied from the TLB Old Area into the ProcBlk TLB New Area’s a1 register.
		proc_new_area->a1=cause;
		
		//Finally, the processor state stored in the ProcBlk as the SYS/Bp New Area is made the current processor state.
		LDST(proc_new_area);
	}else{//non è stata fatta la sys5
		
		terminateProcess(0);
		
	}
}


void bpHandler(){
	
	//Visto che anche qui non sono sicura in che modalità sono, supponiamo di essere in kernel mode, stoppo il tempo utente
	currentProcess->p_userTime += getTODLO() - userTimeStart;
	state_t *sysBp_old = (state_t *) SYSBK_OLDAREA;
	if (!isNull(&currentProcess->p_excpvec[EXCP_SYS_NEW])){//vuol dire che era stata chiamata la sys4
		state_t *proc_new_area = &currentProcess->p_excpvec[EXCP_SYS_NEW];
		state_t *proc_old_area = &currentProcess->p_excpvec[EXCP_SYS_OLD];
		// The processor state is moved from the SYS/Bp Old Area into the processor state stored in the ProcBlk as the SYS/Bp Old Area
		copyState(proc_old_area, sysBp_old);
		
		//the four parameter register (a1-a4) are copied from SYS/Bp Old Area to the ProcBlk SYS/Bp New Area
		proc_new_area->a1=sysBp_old->a1;
		proc_new_area->a2=sysBp_old->a2;
		proc_new_area->a3=sysBp_old->a3;
		proc_new_area->a4=sysBp_old->a4;
		
		//the lower 4 bits of SYS/Bp Old Area’s cpsr register are copied in the most significant positions of ProcBlk SYS/Bp New Area’s a1 register.
		/*questo per ora non lo so fare*/
		
		//Finally, the processor state stored in the ProcBlk as the SYS/Bp New Area is made the current processor state.
		LDST(proc_new_area);
	}else{//non è stata fatta la sys4
		
		terminateProcess(0);
		
	}
	
	
}
