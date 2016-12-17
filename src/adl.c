#include "const.h"
#include "types.h"

typedef struct clist d_list;

typedef struct {
  int *d_semAdd; /* pointer to the semaphore */
  d_list *list_head;
  int d_wake_time;
  int asid;
  d_list d_link;
} delayd_t;



struct d_list adl;
struct d_list delayd_free;

void initADL() {
	static struct delayd_t delayedTable[UPROCMAX];
	adl = CLIST_INIT;
	delayd_free = CLIST_INIT;
    memset(delayedTable, 0, sizeof(struct delayd_t)*UPROCMAX);
    int i;
	for (i = 0; i < UPROCMAX; i++) {
		clist_enqueue(&delayedTable[i], &delayd_free, d_link);
	}
}


// quando qualcuno chiama la syscall delay calcola waketime per passare da
// secondi in millisecondi. Quindi qui ci arriva in ms

int del_insert(int p_asid, int p_waketime) {
  delayd_t *del;

  if (clist_empty(delayd_free)) {
    return TRUE;
  }
    // Allocate the semaphore descriptor
  clist_pop(&delayd_free, del, d_link);

  del->list_head = adl;
  del->d_wake_time = getTODLO() + p_waketime;
  del->asid = p_asid;
  del->d_link = CLIST_INIT;
  clist_insert_asc(del, &adl, d_link, d_wake_time);
  return FALSE;
}


delayd_t *del_head(d_list *list) {
    delayd_t *d_head;
    if (clist_empty(*list))
        d_head = NULL;
    else
        clist_head(d_head, *list, d_link);
    return d_head;
}


void del_remove(d_list *list) {
    delayd_t *del;
    if (!clist_empty(*list)) {
      clist_pop(list, del, d_link);
      memset(del, 0, sizeof(struct delayd_t));
  	  clist_push(del, &delayd_free, d_link);
    }

}
