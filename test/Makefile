.PHONY: clean cleanall ulibclean ulib

UH_DIR = h
UE_DIR = e
ULIB_DIR = ulib
UARM_INCLUDE_DIR=/usr/include/uarm

CC = arm-none-eabi-gcc
CFLAGS = -mcpu=arm7tdmi -nostdlib -fno-builtin-pow -c -Wall -I$(UH_DIR) -I$(UE_DIR)
LD = arm-none-eabi-ld

# Add your test programs here
TESTERS = diskTest fibTest printerTest consumerTest producerTest readTest \
          swapTest todTest helloTest rogueTest multTest prodDiskTest \
					consDiskTest tictactoe

all: totape

testers: $(TESTERS)

convert: $(patsubst %, %.convert, $(TESTERS))

totape: $(patsubst %, %.totape, $(TESTERS))

# Header dependencies
$(patsubst %, %.o, $(TESTERS)): $(UH_DIR)/uconst.h $(UE_DIR)/ulib.e $(UE_DIR)/ulibuarm.e

# Build elf executable
$(TESTERS): %: %.o ulib
	$(LD) -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmaout.x \
	$(ULIB_DIR)/crti.o \
	$(ULIB_DIR)/ulib.o \
	$(ULIB_DIR)/ulibuarm.o \
	$(UARM_INCLUDE_DIR)/libdiv.o \
	$*.o \
	-o $*
	rm $*.o

# Convert from elf to uARM aout format (.aout.uarm)
%.convert: %
	elf2uarm -a $*

# Convert from aout to tape format (.uarm)
%.totape:	%.convert
	uarm-mkdev -t $*.uarm $*.aout.uarm
	rm $*.aout.uarm

cleanall: clean ulibclean

clean:
	rm -f $(patsubst %, %.uarm, $(TESTERS)) \
	      $(patsubst %, %.aout.uarm, $(TESTERS)) \
				$(patsubst %, %.o, $(TESTERS)) \
				$(TESTERS)

ulib:
	cd $(ULIB_DIR) && $(MAKE)

ulibclean:
	cd $(ULIB_DIR) && $(MAKE) clean

