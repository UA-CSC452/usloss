
VERSION=2.12

srcdir = .
prefix = /Users/johartma/cs452/452-students
exec_prefix =   ${prefix}

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


AC_FLAGS =		-DHAVE_CONFIG_H
INSTALL =		/usr/bin/install -c
INSTALL_PROGRAM =	${INSTALL}
INSTALL_DATA =		${INSTALL} -m 644
RANLIB =		@RANLIB@
LN_S =			ln -s
SRC_DIR =		.

CC =			gcc
LD = 			@LD@
LIB_DIR = 		/Users/johartma/cs452/452-students/lib/macosx
INC_DIR	= 	    ${prefix}/include


ifeq ($(shell uname),Darwin)
	CFLAGS += -D_XOPEN_SOURCE
	OS = macosx
	CFLAGS += -Wno-int-to-void-pointer-cast -Wno-extra-tokens -Wno-unused-label -Wno-unused-function
else
	OS = linux
	CFLAGS += -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -Wno-unused-but-set-variable
endif

# Implicit rules for generating object files
.c.o:
	$(CC) $(CFLAGS) $(AC_FLAGS) -DMAKELIB -c -o $*.o $*.c


$(TARGET) : $(COBJS)
	$(AR) -r $@ $(COBJS)

clean:
	rm -f $(COBJS) usloss libusloss*.a core*	


$(COBJS): usloss.h Makefile


install: $(TARGET) 
	mkdir -p $(INC_DIR) $(LIB_DIR)
	$(INSTALL_DATA) usloss.h usyscall.h mmu.h $(INC_DIR)
	$(INSTALL_DATA) $(TARGET) $(LIB_DIR)
# DO NOT DELETE
