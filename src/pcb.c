#include "pcb.h"

extern uint process_c;

struct clist jobs = CLIST_INIT;
struct clist pcbFree;
static pcb_t pcbs[MAXPROC];

pcb_t *findPcb(pid_t pid) {
	pcb_t *pcb;
	void *tmp;
	clist_foreach (pcb, &jobs, p_jobs, tmp) {
		if (pcb->p_pid == pid)
			return pcb;
	}
	return NULL;
}

void initPcbs() {
	memset(pcbs, 0, sizeof(pcb_t)*MAXPROC);
	pcbFree = CLIST_INIT;
	size_t i;
	for (i = 0; i < MAXPROC; i++){
		clist_enqueue(&pcbs[i], &pcbFree, p_list);
	}
}

/*
 * Insert the element pointed to by p onto the pcbFree list.
 */
void freePcb(pcb_t *p) {
	p->p_pid = 0;
	clist_enqueue(p, &pcbFree, p_list);
	clist_delete(p, &jobs, p_jobs);
	process_c--;
}


pcb_t *allocPcb() {
	pcb_t *p;

	if (clist_empty(pcbFree)) {
		p = NULL;
	} else {
		clist_pop(&pcbFree, p, p_list);
		memset(p, 0, sizeof(pcb_t));
		clist_enqueue(p, &jobs, p_jobs);
		process_c++;
	}
	return p;
}

/*
 * Insert the Pcb in the process queue
 */
void insertProcQ(struct clist *q, pcb_t *p) {
	clist_enqueue(p, q, p_list);
}
void insertProcQPriority(struct clist *q, pcb_t *p) {
	clist_push(p, q, p_list);
}

//Remove from head
pcb_t *removeProcQ(struct clist *q) {
	pcb_t *p;
	if (clist_empty(*q)) {
		p = NULL;
	} else {
		clist_pop(q, p, p_list);
		p->p_list = CLIST_INIT;
	}
	return p;
}

//Search and remove
pcb_t *outProcQ(struct clist *q, pcb_t *p) {
	byte not_found = clist_delete(p, q, p_list);
	if (!not_found)
		p->p_list = CLIST_INIT;
	return not_found ? NULL : p;
}

pcb_t *headProcQ(struct clist *q) {
	pcb_t *p;
	if (clist_empty(*q))
		p = NULL;
	else
		clist_head(p, *q, p_list);
	return p;
}

//Check if the Pcb has children
int emptyChild(pcb_t *p) {
	return clist_empty(p->p_children);
}

void insertChild(pcb_t *parent, pcb_t *p) {
	p->p_parent = parent;
	clist_enqueue(p, &parent->p_children, p_siblings);
}

pcb_t *removeChild(pcb_t *p) {
	pcb_t *child;
	if (emptyChild(p))
		child = NULL;
	else {
		clist_pop(&p->p_children, child, p_siblings);
		child->p_siblings = CLIST_INIT;
		child->p_parent = NULL;
	}
	return child;
}

pcb_t *outChild(pcb_t *p) {
	if (!p->p_parent)
		return NULL;
	else {
		byte not_found = clist_delete(p, &p->p_parent->p_children, p_siblings);
		if (!not_found) {
			p->p_siblings = CLIST_INIT;
			p->p_parent = NULL;
		}
		return not_found ? NULL : p;
	}
}
