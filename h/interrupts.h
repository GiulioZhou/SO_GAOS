#ifndef INTERR_H_INCLUDED
#define INTERR_H_INCLUDED


extern cputime_t interStart;

int device_numb(memaddr *pending);
void intHandler();
void intDev(int int_no);
void intTerm();
void intTimer();

#endif /* SCHED_H_INCLUDED */
