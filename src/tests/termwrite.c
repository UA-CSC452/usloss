
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "usloss.h"

char *text = "Hello World!";
int i = 0;

void 
term_handler(int type, void *arg) {
    int 	result;
    int 	status;
    char 	ch;
    int		unit = (int) arg;

    USLOSS_Console("interrupt\n");
    if (i >= strlen(text)) {
        USLOSS_Halt(0);
    }
    int control = 0;
    control = USLOSS_TERM_CTRL_CHAR(control, text[i]);
    control = USLOSS_TERM_CTRL_XMIT_INT(control);
    control = USLOSS_TERM_CTRL_RECV_INT(control);
    control = USLOSS_TERM_CTRL_XMIT_CHAR(control);
    int out = USLOSS_DeviceOutput(USLOSS_TERM_DEV, unit, (void*) control);
    assert (out == USLOSS_DEV_OK);
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
startup()
{
    int	status;
    int i;

    for (i = 0; i < USLOSS_NUM_INTS; i++) {
	   USLOSS_IntVec[i] = dummy_handler;
    }
    USLOSS_IntVec[USLOSS_TERM_INT] = term_handler;
    int control = 0;
    control = USLOSS_TERM_CTRL_RECV_INT(control);
    control = USLOSS_TERM_CTRL_XMIT_INT(control);
    status = USLOSS_DeviceOutput(USLOSS_TERM_DEV, 1, (void *) control);
    // Turn on interrupts.
    USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    // Wait in an infinite loops for interrupts.
    while(1) {
	   USLOSS_WaitInt();
    }
}

void
finish() {}
