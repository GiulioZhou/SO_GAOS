#include "asl.h"


// Active Semaphores List
struct clist aslh;

// Free Semaphores List
struct clist semdFree;

/*
 * Initialize the semdFree list to contain all the elements of the array
 * static struct semd_t semdTable[MAXPROC]
 * This method will be only called once during data structure in initialization.
 */
void initASL() {
	static struct semd_t semdTable[MAXPROC];
	aslh = CLIST_INIT;
	semdFree = CLIST_INIT;
    memset(semdTable, 0, sizeof(struct semd_t)*MAXPROC);
    int i;
	for (i = 0; i < MAXPROC; i++) {
		clist_enqueue(&semdTable[i], &semdFree, s_link);
	}
}

/*
 * Search a semaphore in the aslh given its semAdd, returns the semaphore if it
 * is found, NULL otherwise.
 */
struct semd_t *searchAsl(int *semAdd) {
	void *tmp;
	struct semd_t *scan, *ret = NULL;
	clist_foreach(scan, &aslh, s_link, tmp) {
		if (scan->s_semAdd == semAdd) {
			ret = scan;
			break;
		}
    }
	return ret;
}

/*
 * Insert the ProcBlk pointed to by p at the tail of the process queue associated
 * with the semaphore whose physical address is semAdd and set the
 * semaphore address of p to semAdd. If the semaphore is currently not active
 * (i.e. there is no descriptor for it in the ASL), allocate a new descriptor
 * from the semdFree list, insert it in the ASL (at the appropriate position),
 * initialize all of the fields (i.e. set s_semAdd to semAdd , and s_procq), and
 * proceed as above. If a new semaphore descriptor needs to be allocated
 * and the semdFree list is empty, return TRUE. In all other cases return
 * FALSE
 */
int insertBlocked(int *semAdd, pcb_t *p) {
	struct semd_t *semd = searchAsl(semAdd);
	if (!semd) {
		// Return 1 if semdFree is empty (error)
		if (clist_empty(semdFree))
			return TRUE;

		// Allocate the semaphore descriptor
		clist_pop(&semdFree, semd, s_link);
		semd->s_semAdd = semAdd;
        semd->s_link = CLIST_INIT;
		semd->s_procq = CLIST_INIT;
		clist_insert_asc(semd, &aslh, s_link, s_semAdd);
	}
	insertProcQ(&(semd->s_procq), p);
	p->p_cursem = semd;
	return FALSE;
}

/*
 *Search the ASL for a descriptor of this semaphore. If none is found, return
 *NULL; otherwise, remove the first (i.e. head) ProcBlk from the process
 *queue of the found semaphore descriptor and return a pointer to it. If the
 *process queue for this semaphore becomes empty remove the semaphore
 *descriptor from the ASL and return it to the semdFree list.
 */
pcb_t *removeBlocked(int *semAdd) {
	struct semd_t *semd = searchAsl(semAdd);
	if (!semd) return NULL;

	pcb_t *p = removeProcQ(&semd->s_procq);
	if (!p) return NULL; // This is (hopefully) never reached since a semaphore cannot have an empty proc queue

	if (clist_empty(semd->s_procq)) {
		clist_delete(semd, &aslh, s_link);
        semd->s_link = CLIST_INIT;
		clist_push(semd, &semdFree, s_link);
	}

	return p;
}

/*
 * Remove the ProcBlk pointed to by p from the process queue associated
 * with p’s semaphore on the ASL. If ProcBlk pointed to by p does not
 * appear in the process queue associated with p’s semaphore, which is an
 * error condition, return NULL; otherwise, return p.
 */
pcb_t *outBlocked(pcb_t *p) {
	return !p->p_cursem || !outProcQ(&p->p_cursem->s_procq, p) ? NULL : p;
}

/*
 * Return a pointer to the ProcBlk that is at the head of the process queue
 * associated with the semaphore semAdd. Return NULL if semAdd is not
 * found on the ASL or if the process queue associated with semAdd is empty.
*/
pcb_t *headBlocked(int *semAdd) {
	struct semd_t *semd = searchAsl(semAdd);
	if (!semd)
		return NULL;
	return headProcQ(&semd->s_procq);
}


/*
 * Wake up processes locked on a semaphore and put them in the process queue
 */
// pcb_t *wakeupBlocked(int *semaddr) {
//     pcb_t *p = headBlocked(semaddr);
//     if (p && *semaddr >= p->p_resources_needed) {
//         *semaddr-= p->p_resources_needed;
//         p->p_resources_needed = 0;
//         removeBlocked(semaddr);
//         insertProcQ(&ready_queue, p);
// 		return p;
//     }
// 	return NULL;
// }
