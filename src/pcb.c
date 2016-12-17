#include "pcb.h"

extern uint process_c;

struct clist jobs = CLIST_INIT;
struct clist pcbFree;

pcb_t *pcbMemory() {
    static pcb_t procBlk[MAXPROC];
    return procBlk;
}

/*
 * Find a pcb given its id
 */
pcb_t *findPcb(pid_t pid) {
    pcb_t *pcb;
    void *tmp;
    clist_foreach (pcb, &jobs, p_jobs, tmp) {
        if (pcb->p_pid == pid)
            return pcb;
    }
    return NULL;
}


/*
 * Initialize the pcbFree list to contain all the elements of the static array
 * of MAXPROC ProcBlk’s. This method will be called only once during data
 * structure initialization.
 */
void initPcbs() {
    pcb_t *pcbs = pcbMemory();
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


/*
 * Return NULL if the pcbFree list is empty. Otherwise, remove an element
 * from the pcbFree list, provide initial values for ALL of the ProcBlk’s
 * fields (i.e. fill the entire structure by NULL/zero bytes) and then return
 * a pointer to the removed element. ProcBlk’s get reused, so it is important
 * that no previous value persist in a ProcBlk when it gets reallocated.
 * There is still the question of how one acquires storage for MAXPROC
 * ProcBlk’s and gets these MAXPROC ProcBlk’s initially onto the pcbFree
 * list. Unfortunately, there is no malloc() feature to acquire dynamic (i.e.
 * non-automatic) storage that will persist for the lifetime of the OS and not
 * just the lifetime of the function they are declared in. Instead, the storage
 * for the MAXPROC ProcBlk’s will be allocated as static storage. A static
 * array of MAXPROC ProcBlk’s will be declared in initPcbs(). Furthermore,
 * this method will insert each of the MAXPROC ProcBlk’s onto the
 * pcbFree list.
 */
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
 * Insert the ProcBlk pointed to by p into the process queue whose list-tail
 * pointer is q.
 */
void insertProcQ(struct clist *q, pcb_t *p) {
    clist_enqueue(p, q, p_list);
}

/*
 * Remove the first (i.e. head) element from the process queue whose listtail
 * pointer is q. Return NULL if the process queue was initially empty;
 * otherwise return the pointer to the removed element.
 */
 //REVIEW: non dovrebbe metterlo nei pcbFree??
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

/*
 * Remove the ProcBlk pointed to by p from the process queue whose list-tail
 * pointer is q. If the desired entry is not in the indicated queue (an error
 * condition), return NULL; otherwise, return p. Note that p can point to
 * any element of the process queue.
 */
pcb_t *outProcQ(struct clist *q, pcb_t *p) {
    byte not_found = clist_delete(p, q, p_list);
    if (!not_found)
        p->p_list = CLIST_INIT;
    return not_found ? NULL : p;
}

/*
 * Return a pointer to the first ProcBlk from the process queue whose listtail
 * pointer is q. Do not remove this ProcBlk from the process queue.
 * Return NULL if the process queue is empty.
 */
pcb_t *headProcQ(struct clist *q) {
    pcb_t *p;
    if (clist_empty(*q))
        p = NULL;
    else
        clist_head(p, *q, p_list);
    return p;
}

/*
 * Return TRUE if the ProcBlk pointed to by p has no children. Return
 * FALSE otherwise.
 */
int emptyChild(pcb_t *p) {
    return clist_empty(p->p_children);
}

/*
 * Make the ProcBlk pointed to by p a child of the ProcBlk pointed to by parent.
 */
void insertChild(pcb_t *parent, pcb_t *p) {
    p->p_parent = parent;
    clist_enqueue(p, &parent->p_children, p_siblings);
}

/*
 * Make the first child of the ProcBlk pointed to by p no longer a child of p.
 * Return NULL if initially there were no children of p. Otherwise, return a
 * pointer to this removed first child ProcBlk.
 */
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

/*
 * Make the ProcBlk pointed to by p no longer the child of its parent. If the
 * ProcBlk pointed to by p has no parent, return NULL; otherwise, return
 * p. Note that the element pointed to by p need not be the first child of its
 * parent.
 */
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
