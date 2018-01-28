#include "const.h"
#include "types.h"
#include "util.h"
#include <libuarm.h>

struct clist adl = CLIST_INIT;
struct clist delayd_free = CLIST_INIT;

void initADL() {
	int i;
	static struct delayd_t delayedTable[UPROCMAX];

	memset(delayedTable, 0, sizeof(struct delayd_t)*UPROCMAX);
	for (i = 0; i < UPROCMAX; i++) {
		delayedTable[i].d_sem_index = i;
		clist_enqueue(&delayedTable[i], &delayd_free, d_link);
	}
}

int del_insert(int p_asid, int p_waketime) {
	delayd_t *del;
	clist_pop(&delayd_free, del, d_link);
	if (!del) return -1;

	del->d_wake_time = getTODLO() + p_waketime;
	del->asid = p_asid;
	del->d_link = CLIST_INIT;
	clist_insert_asc(del, &adl, d_link, d_wake_time);
	return del->d_sem_index;
}

int del_remove() {
	delayd_t *del;
	if((del = clist_head(del, adl, d_link)) && del->d_wake_time < getTODLO()) {
		clist_pop(&adl, del, d_link);

		del->d_wake_time = 0;
		del->asid = 0;
		del->d_link = CLIST_INIT;

		clist_push(del, &delayd_free, d_link);
		return del->d_sem_index;
	}
	return -1;
}
