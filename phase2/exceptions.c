#include <asl.h>
#include <pcb.h>
#include <initial.h>
#include <scheduler.h>
#include <exceptions.h>
#include <systemcall.h>

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
		//NON VA BENE...
			currentProcess->p_userTime += getTODLO() - userTimeStart;
			
			
			
			/* Gestisce ogni singola SYSCALL */
			switch(sysc)
			{
				case CREATEPROCESS:
					currentProcess->p_s.a1 = createProcess((state_t *) argv1); // se questo cast non funziona provare a fare argv1 di tipo U32
					break;
					
				case TERMINATEPROCESS:
					terminateProcess((pid_t) argv1);
					
					break;
					
				case SEMOP:
				//PROBLEMA: se tolgo questa tprint non funziona più niente >.>
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
					currentProcess->p_s.a1 = ioDevOp((unsigned int) argv1, (int) argv2, (unsigned int) argv3);
					break;
					
				case GETPID:
					currentProcess->p_s.a1 = getPid();
					break;
					
					
					
				default:
					bpHandler();
			}
			
			
			if(currentProcess)
				LDST(&currentProcess->p_s);
			else
				scheduler();
			
		}
		else if ((currentProcess->p_s.cpsr & STATUS_USER_MODE) == STATUS_USER_MODE) {//qui nel caso sono in user mode e provo a fare una syscall faccio come mi dicono le specifiche, copiando le old aree giuste e alzando una trap chiamando l'handler delle trap
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
		PANIC();
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
