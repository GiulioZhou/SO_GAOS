#ifndef _DEVICES_H
#define _DEVICES_H

#include "types.h"
#include "asl.h"
#include "syscalls.h"

void handlePseudoClock();
void handleDevice(int device, int dev_sem[]);
void handleTimer();
void handleTerminal();

#endif
