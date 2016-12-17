#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include "const.h"
#include "types.h"
#include "util.h"
#include "asl.h"
#include "scheduler.h"
#include "syscalls.h"
#include "devices.h"
#include <libuarm.h>

void excHandleInterrupt();
void excHandleTLB();
void excHandlePGMT();
void excHandleSYSBP();

#endif