# Cartelle contenenti file da compilare e nome del file finale
TARGET := kernel
SRC_PATH := src
SRC_EXT := c
BUILD_PATH:= build

# Librerie da linkare
LIBS :=

# Flags per compilatore e linker
CFLAGS := -c -mcpu=arm7tdmi -I /usr/include/uarm/
LFLAGS := -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x
LDARGS := /usr/include/uarm/libuarm.o /usr/include/uarm/crtso.o

# Lista di file da compilare
SOURCES = $(shell find $(SRC_PATH) -name '*.$(SRC_EXT)')
OBJECTS = $(SOURCES:$(SRC_PATH)/%.$(SRC_EXT)=$(BUILD_PATH)/%.o)

PREFIX = arm-none-eabi
CC = $(PREFIX)-gcc
LD = $(PREFIX)-ld

debug: CFLAGS += -DDEBUG
debug: all

# Regola all
all: dirs
all: $(TARGET).uarm

# Regola per la generazione dei file per uArm
%.uarm: %.elf
	@echo Generating .uarm file...
	elf2uarm -k $<

# Regola per la generazione dei file ELF
$(TARGET).elf: $(OBJECTS)
	@echo Creating .elf file $@...
	$(LD) $(LFLAGS) -o $@ $(LDARGS) $^

# Regola per la generazione dei file object
$(BUILD_PATH)/%.o: $(SRC_PATH)/%.$(SRC_EXT)
	@echo Creating object file $@...
	$(CC) $(LIBS) $(CFLAGS) -c $< -o $@


# Regola clean
clean:
	@echo "Deleting directories"
	@$(RM) -r $(BUILD_PATH)


# Create the directories used in the build
.PHONY: dirs
dirs:
	@echo "Creating directories"
	@mkdir -p $(dir $(OBJECTS))
