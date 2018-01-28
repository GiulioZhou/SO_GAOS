#ifndef UARM_ULIBUARM_H
#define UARM_ULIBUARM_H

/*****************************************************************************
 * Functions valid in user mode                                              *
 *****************************************************************************/

/* This function cause a system call trap */
unsigned int SYSCALL(unsigned int number, unsigned int arg1, unsigned int arg2, unsigned int arg3);

#endif
