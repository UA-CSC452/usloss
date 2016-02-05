
VERSION=2.11
CC=gcc
CFLAGS=-g -I.. -Wall -L..
CFLAGS += -Wno-int-to-void-pointer-cast
LDFLAGS=-L.. -v
LIB=-lusloss$(VERSION)
TESTS = psr psr2 psr3 clock context interrupt syscall mmu syscall2 mmu2 context0 mmu3

ifeq ($(shell uname),Darwin)
	CFLAGS += -D_XOPEN_SOURCE
	OS = macosx
else
	OS = linux
endif

all: $(TESTS)

$(TESTS) : %: %.c stubs.o ../libusloss$(VERSION).a
	$(CC) $(CFLAGS) $< stubs.o -o $@ $(LIB)

clean:
	rm -f *.o $(TESTS) core* term?.out
