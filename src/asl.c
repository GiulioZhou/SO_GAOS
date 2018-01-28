#include "asl.h"

struct clist aslh[2]; // Active Semaphores List
struct clist semdFree[2]; // Free Semaphores List

/* Initialize the semdFree[0] semdTable */
static struct semd_t semdTable[MAXPROC];
void initASL() {
	aslh[0] = CLIST_INIT;
	semdFree[0] = CLIST_INIT;
	memset(semdTable, 0, sizeof(struct semd_t)*MAXPROC);
	int i;
	for (i = 0; i < MAXPROC; i++) {
		clist_enqueue(&semdTable[i], &semdFree[0], s_link);
	}
}

/* Initialize the semdFree[1] virtSemdTable */
static struct semd_t virtSemdTable[UPROCMAX];
void initAVSL() {
	aslh[1] = CLIST_INIT;
	semdFree[1] = CLIST_INIT;
	memset(virtSemdTable, 0, sizeof(struct semd_t)*UPROCMAX);
	int i;
	for (i = 0; i < UPROCMAX; i++) {
		clist_enqueue(&virtSemdTable[i], &semdFree[1], s_link);
	}
}

//virt == 1 if the semaphore is virtual
struct semd_t *searchAsl(int *semAdd, int virt) {
  virt = virt ? 1 : 0;
	struct  clist *tmp, *head;
	struct semd_t *scan, *ret = NULL;

	head = &aslh[virt];

	clist_foreach(scan, head, s_link, tmp) {
		if (scan->s_semAdd == semAdd) {
			ret = scan;
			break;
		}
	}
	return ret;
}

int insertBlocked(int *semAdd, pcb_t *p, int virt) {
  virt = virt ? 1 : 0;
	struct semd_t *semd = searchAsl(semAdd, virt);
	if (!semd) {

		if (clist_empty(semdFree[virt]))
			return TRUE;

		// Allocate the semaphore descriptor
		semd = NULL;

		clist_pop(&semdFree[virt], semd, s_link);
		if (!semd) {
			fatal("semdFree is empty\n");
		}

		semd->s_semAdd = semAdd;
		semd->s_link = CLIST_INIT;
		semd->s_procq = CLIST_INIT;
		clist_insert_asc(semd, &aslh[virt], s_link, s_semAdd);
	}
	insertProcQ(&(semd->s_procq), p);
	p->p_cursem = semd;
	return FALSE;
}

pcb_t *removeBlocked(int *semAdd, int virt) {
  virt = virt ? 1 : 0;
	struct semd_t *semd = searchAsl(semAdd, virt);
	if (!semd) return NULL;

	pcb_t *p = removeProcQ(&semd->s_procq);
	if (!p) return NULL; // This is (hopefully) never reached since a semaphore cannot have an empty proc queue

	if (clist_empty(semd->s_procq)) {
		clist_delete(semd, &aslh[virt], s_link);
		semd->s_link = CLIST_INIT;
		clist_push(semd, &semdFree[virt], s_link);
	}

	return p;
}


pcb_t *outBlocked(pcb_t *p) {
	return !p->p_cursem || !outProcQ(&p->p_cursem->s_procq, p) ? NULL : p;
}


pcb_t *headBlocked(int *semAdd, int virt) {
  virt = virt ? 1 : 0;
	struct semd_t *semd = searchAsl(semAdd, virt);
	if (!semd)
		return NULL;
	return headProcQ(&semd->s_procq);
}
