# Makefile to build the user-mode libraries needed for u-procs: a minimal
# libumps (called ulibumps), that only has the SYSCALL interface, and the
# crti library (for u-proc initialization).

.PHONY: clean

UE_DIR = ../e
UH_DIR = ../h

CC = arm-none-eabi-gcc 
CFLAGS = -mcpu=arm7tdmi -fno-builtin-pow -c -pedantic -Wall -I$(UE_DIR) -I$(UH_DIR)
SFLAGS = -fPIC -c

all: ulibuarm.o crti.o ulib.o

crti.o: crti.s
	$(CC) $(SFLAGS) crti.s -o crti.o

ulibuarm.o: ulibuarm.s
	$(CC) $(SFLAGS) ulibuarm.s -o ulibuarm.o

ulib.o: $(UH_DIR)/uconst.h $(UE_DIR)/ulib.e $(UE_DIR)/ulibuarm.e

clean:
	rm -f crti.o ulibuarm.o ulib.o
