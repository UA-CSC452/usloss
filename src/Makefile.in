
include ../version.mk
include ../config.mk

# List of object files to generate (and the list of source files, generated
# by pattern substitution)

COBJS=main.o globals.o devices.o dev_disk.o dev_term.o dev_alarm.o dev_clock.o \
	sig_ints.o mmu.o
SRCS=${COBJS:.o=.c}

LIBS= -lusloss$(VERSION)

CFLAGS=-Wall -DVERSION=\"$(VERSION)\" 
CFLAGS += -DMMU
CFLAGS += -Wno-deprecated-declarations 
CFLAGS += -Wno-pointer-to-int-cast
CFLAGS += -Wno-int-to-pointer-cast
CFLAGS += -Werror
#CFLAGS += -DDEBUG
CFLAGS += -g
CFLAGS += -DVIRTUAL_TIME
LD = ld
LDFLAGS = -L.
TARGET=libusloss$(VERSION).a


TESTS = $(patsubst %.c,%,$(wildcard tests/*.c))
TOBJS = ${TESTS:=.o}
CFLAGS += -I.


ifeq ($(shell uname),Darwin)
	CFLAGS += -D_XOPEN_SOURCE
	OS = macosx
	CFLAGS += -Wno-int-to-void-pointer-cast -Wno-extra-tokens -Wno-unused-label -Wno-unused-function
	LIBFLAGS = -Wl,-all_load $(LIBS)
	# The following only works for dynamic libraries. TODO make it work with static.
	LDFLAGS += -Wl,-warn_commons -Wl,-fatal_warnings
else
	OS = linux
	CFLAGS += -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -Wno-unused-but-set-variable
	LIBFLAGS = -Wl,--start-group $(LIBS) -Wl,--end-group
	LDFLAGS +=  -Wl,--warn-common -Wl,--fatal-warnings
endif

# Implicit rules for generating object files
.c.o:
	$(CC) $(CFLAGS) $(AC_FLAGS) -DMAKELIB -c -o $*.o $*.c

all: $(TARGET) tests

$(TARGET) : $(COBJS)
	$(AR) -r $@ $(COBJS)

tests: $(TESTS)

$(TESTS):   %: $(TARGET) %.o Makefile
	- $(CC) $(LDFLAGS) -o $@ $@.o $(LIBFLAGS)

clean:
	rm -f $(COBJS) $(TOBJS) $(TESTS) usloss *.a core* term*.out disk[01]

distclean: clean
	rm -rf Makefile config.h config.log config.status config.mk autom4te.cache


$(COBJS): usloss.h Makefile


install: $(TARGET) 
	mkdir -p $(INC_DIR) $(LIB_DIR)
	$(INSTALL_DATA) usloss.h usyscall.h $(INC_DIR)
	$(INSTALL_DATA) $(TARGET) $(LIB_DIR)
# DO NOT DELETE
