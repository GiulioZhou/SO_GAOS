#include "syscalls.h"

extern pcb_t *current_process;

int write_sem = 1;


void sys13_WRITE_FROM_TERMINAL(int a2, int a3, int a4) {

    sys10_SEMOP((int) &write_sem, -1, 0);

    if (current_process) {
        char *s = (char *) a2;
        int i = 0;

        while (a3) {
            /* Put "transmit char" command+char in term0 register (3rd word). This
                actually starts the operation on the device! */
            command = PRINTCHR | (((devregtr)*s) << BYTELEN);

            /* Wait for I/O completion (SYS8) */
            status = sys10_IODEVOP(command, INT_TERMINAL, 0);

            if (!current_process)

            if ((status & TERMSTATMASK) != TRANSM) {
                PANIC();
            }

            if (((status & TERMCHARMASK) >> BYTELEN) != *s) {
                PANIC();
            }

            s++;
            a3--;
        }
    }

    sys10_SEMOP((int) &write_sem, 1, 0);

}