
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "usloss.h"

char *text = "Hello World!";

void
writechar(int unit, char ch) {
    int control = 0;
    control = USLOSS_TERM_CTRL_CHAR(control, ch);
    control = USLOSS_TERM_CTRL_XMIT_INT(control);
    control = USLOSS_TERM_CTRL_RECV_INT(control);
    control = USLOSS_TERM_CTRL_XMIT_CHAR(control);
    int out = USLOSS_DeviceOutput(USLOSS_TERM_DEV, unit, (void*) control);
    assert (out == USLOSS_DEV_OK);
    return;
}

void 
term_handler(int type, void *arg) {
    int		unit = (int) arg;
    static int i = 1;

    USLOSS_Console("term_handler %d %c 0x%x\n", i, text[i], text[i]);
    if (i >= strlen(text)) {
        USLOSS_Halt(0);
    }
    writechar(unit, text[i]);
    i++;
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

    for (i = 0; i < USLOSS_NUM_INTS; i++) {
	   USLOSS_IntVec[i] = dummy_handler;
    }
    USLOSS_IntVec[USLOSS_TERM_INT] = term_handler;
    // Turn on interrupts.
    status = USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    if (status != USLOSS_ERR_OK) {
        USLOSS_Halt(1);
    }
    writechar(1, text[0]);
    // Wait in an infinite loops for interrupts.
    while(1) {
	   USLOSS_WaitInt();
    }
}

void
finish(int argc, char **argv) {}
void test_setup(int argc, char **argv) {}
void test_cleanup(int argc, char **argv) {}