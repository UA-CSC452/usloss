
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "usloss.h"

#define SIZE 1024

char output[SIZE];

int counts[USLOSS_TERM_UNITS];

void 
term_handler(int type, void *arg) {
    int		unit = (int) arg;

    USLOSS_Console("interrupt\n");
    if (counts[unit] >= SIZE) {
        USLOSS_Halt(0);
    }
    int control = 0;
    control = USLOSS_TERM_CTRL_CHAR(control, output[counts[unit]++]);
    control = USLOSS_TERM_CTRL_XMIT_INT(control);
    control = USLOSS_TERM_CTRL_RECV_INT(control);
    control = USLOSS_TERM_CTRL_XMIT_CHAR(control);
    int out = USLOSS_DeviceOutput(USLOSS_TERM_DEV, unit, (void*) control);
    assert (out == USLOSS_DEV_OK);
}

//
// This is a dummy handler for interrupts we don't care about, e.g.
// the clock interrupt. 
//
void
dummy_handler(int type, void *arg)
{
    // Do nothing.
}

void
startup(int argc, char **argv)
{
    int	status;
    int i;

    for (i = 0; i < SIZE; i++) {
        output[i] = 'A' + (i % 26);
    }

    for (i = 0; i < USLOSS_NUM_INTS; i++) {
	   USLOSS_IntVec[i] = dummy_handler;
    }
    USLOSS_IntVec[USLOSS_TERM_INT] = term_handler;
    int control = 0;
    control = USLOSS_TERM_CTRL_RECV_INT(control);
    control = USLOSS_TERM_CTRL_XMIT_INT(control);
    for (i = 0; i < USLOSS_TERM_UNITS; i++) {
        counts[i] = 0;
        status = USLOSS_DeviceOutput(USLOSS_TERM_DEV, i, (void *) control);
        assert(status == USLOSS_DEV_OK);
    }
    // Turn on interrupts.
    status = USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    assert(status == USLOSS_ERR_OK);
    // Wait in an infinite loops for interrupts.
    while(1) {
	   USLOSS_WaitInt();
    }
    // Never gets here.
}

void
finish(int argc, char **argv) {}

void 
test_setup(int argc, char **argv) {}

void
test_cleanup(int argc, char **argv) 
{
    FILE    *f;
    char input[SIZE];
    char name[50];
    int     n;
    int     i;

    for(i = 0; i < USLOSS_TERM_UNITS; i++) {
        memset(input, '\0', SIZE);
        snprintf(name, sizeof(name), "term%d.out", i);
        f = fopen(name, "r");
        assert(f != NULL);
        n = fread(input, 1, SIZE, f);
        assert(n == SIZE);
        fclose(f);
        assert(!memcmp(input, output, SIZE));
    }
}
