#include <pcb.h>
#include <initial.h>
#include <scheduler.h>
#include <const.h>

#include <libuarm.h>
#include <arch.h>

unsigned int time_slice_start = 0;		//ultima partenza del time slice
unsigned int userTimeStart = 0;			//per contare lo userTime del processo corrente
unsigned int CPUTimeStart = 0;			//per contare il CPUTime del processo corrente
unsigned int pseudo_clock_start = 0;	//ultima partenza dello pseudo clock
unsigned int current_timer;				//quale dei due timer ( pseudo clock o time slice ) alzerà un interrupt per primo

/*
void scheduler(){
	
	setTIMER(0);
	currentProcess = removeProcQ(&readyQueue);
	time_slice_start=getTODLO();
	setTIMER(getTIMER()+0xd900);
	
	LDST( &currentProcess->p_s );
	
}


*/


void scheduler(){
	//RIVEDERE se è garantito che lo pseudo clock scatta ogni 100 millisecondi
	unsigned int time = getTODLO(); // mi salvo quando sono entrato nello scheduler
	//controllo se è finito il time slice o lo pseudo-clock, se uno dei due è finito, vuol dire che lo scheduler è stato chiamaato
	//dall'interrupt del Timer
	int slice_end = SCHED_TIME_SLICE - (time - time_slice_start );//tempo che manca alla fine del time slice corrente
	int clock_end = SCHED_PSEUDO_CLOCK - (time - pseudo_clock_start);//tempo che manca alla fine dello pseudo clock tick corrente
	
	if( slice_end <= 0 || currentProcess==NULL && slice_end > 0 ){ //time slice terminato o metto in escuzione un nuovo processo, setta il prossimo
		time_slice_start = time;
		slice_end = SCHED_TIME_SLICE;
		tprint("time slice terminato\n");
	}
	
	if( clock_end <=0 ){	//pseudo clock terminato, setta il prossimo
		pseudo_clock_start = time;
		clock_end = SCHED_PSEUDO_CLOCK;
		tprint("pseudo clock terminato\n");
	}
	
	if( clock_end <= slice_end ){	//salvo quale timer è stato settato -> Ci serve saperlo?
		setTIMER(clock_end);
		current_timer = PSEUDO_CLOCK;
	}
	else{
		setTIMER(slice_end);
		current_timer = TIME_SLICE;
	}

	if (currentProcess==NULL){
		if( !clist_empty(readyQueue) ){
			currentProcess = removeProcQ(&readyQueue);
			tprint("impostato currentProcess\n");
		}
		else{
			if( processCount == 0 )	//non ci sono più processi e posso terminare
				HALT();
			else if( (processCount > 0) && (softBlockCount == 0 ))	//deadlock
				PANIC();
			else if(processCount > 0 && softBlockCount > 0)
				WAIT();
			else
				PANIC();
		}
		CPUTimeStart = getTODLO();	//se comincia l'esecuzione di un nuovo processo riparte il conteggio del tempo di CPU
	}
	
	userTimeStart = getTODLO();	//riparte il conteggio del tempo utente perché noi entriamo nello scheduler come kernel quindi dobbiamo far ripartire il tempo utente quando usciamo
	
	LDST( &currentProcess->p_s );	//carico nel processore lo stato del processo scelto come prossimo

}


