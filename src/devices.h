#ifndef _DEVICES_H
#define _DEVICES_H

#include "types.h"
#include "asl.h"
#include "syscalls.h"

void handlePseudoClock();
void handleTimer();
void handleTerminal();

#endif