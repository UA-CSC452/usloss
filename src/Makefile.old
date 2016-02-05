
VERSION=2.11

# List of object files to generate (and the list of source files, generated
# by pattern substitution)

COBJS=main.o globals.o devices.o dev_disk.o dev_term.o dev_alarm.o dev_clock.o \
	sig_ints.o mmu.o
SRCS=${COBJS:.o=.c}
CC=gcc
CFLAGS=-Wall -DVERSION=\"$(VERSION)\" 
CFLAGS += -DMMU
CFLAGS += -Wno-deprecated-declarations 
CFLAGS += -Wno-pointer-to-int-cast
CFLAGS += -Wno-int-to-pointer-cast
#CFLAGS += -DDEBUG
CFLAGS += -g
CFLAGS += -DVIRTUAL_TIME
LD = ld
LDFLAGS = -L.
TARGET=libusloss$(VERSION).a

ifndef CS452_STUDENTS
CS452_STUDENTS = $(HOME)/Dropbox/452-students
endif

INSTALL = $(CS452_STUDENTS)


ifeq ($(shell uname),Darwin)
	DEFINES += -D_XOPEN_SOURCE
	OS = macosx
	CFLAGS += -Wno-int-to-void-pointer-cast -Wno-extra-tokens -Wno-unused-label -Wno-unused-function
else
	OS = linux
	CFLAGS += -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -Wno-unused-but-set-variable
endif

ifeq ($(shell uname),Darwin)
    # Add a few things for the Mac
    CFLAGS += -D_XOPEN_SOURCE
    CFLAGS += -Wno-int-to-void-pointer-cast
    OS = macosx
else
	OS = linux
endif

# Implicit rules for generating object files
.c.o:
	$(CC) $(CFLAGS) -DMAKELIB -c -o $*.o $*.c


$(TARGET) : $(COBJS)
	$(AR) -r $@ $(COBJS)

clean:
	rm -f $(COBJS) usloss libusloss*.a core*	


$(COBJS): usloss.h Makefile


install: $(TARGET) 
	mkdir -p $(INSTALL)/include $(INSTALL)lib
	cp usloss.h usyscall.h mmu.h $(INSTALL)/include
	-chmod ug+w,o+r $(INSTALL)/include/*
	cp $(TARGET) $(INSTALL)/lib/$(OS)
	-chmod ug+w,o+r $(INSTALL)/lib/$(OS)/$(TARGET)
	-rm -f $(INSTALL)/lib/$(OS)/libusloss.a
	-(cd $(INSTALL)/lib/$(OS); ln -s $(TARGET) libusloss.a)
# DO NOT DELETE
