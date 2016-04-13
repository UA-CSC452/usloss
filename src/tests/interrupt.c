// This is an example of how USLOSS interrupts work. It simply reads
// characters from terminal 0, and copies them to the USLOSS_Console. You
// need to create a file called term0.in in the same directory that
// you run this program, containing the characters you want copied.
// This program runs in an infinite loop so you'll have to kill it
// when you get tired of watching it run.

#include <stdio.h>
#include "usloss.h"

//
// Read characters from terminal 0 and writes them to the USLOSS_Console.
//
void 
term_handler(int type, void *arg) {
    int 	result;
    int 	status;
    char 	ch;
    int		unit = (int) arg;

    if (unit != 0) {
	return;
    }
    // Get the device status for terminal 0.
    result = USLOSS_DeviceInput(USLOSS_TERM_DEV, 0, &status);
    if (result != USLOSS_DEV_OK) {
	USLOSS_Console("Something is wrong with terminal 0!!");
	USLOSS_Halt(1);
    }
    // If the status is "USLOSS_DEV_BUSY" then a character has been received.
    if (USLOSS_TERM_STAT_RECV(status) == USLOSS_DEV_BUSY) {
	ch = USLOSS_TERM_STAT_CHAR(status);	// Get the character
	USLOSS_Console("%c", ch);
    }
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
    // Turn on receive interrupts for terminal 0.
    status = USLOSS_DeviceOutput(USLOSS_TERM_DEV, 0, (void *) USLOSS_TERM_CTRL_RECV_INT(0));
    USLOSS_IntVec[USLOSS_TERM_INT] = term_handler;
    // Turn on interrupts.
    USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    // Wait in an infinite loops for interrupts.
    while(1) {
	USLOSS_WaitInt();
    }
}

void
finish() {}
void setup(void) {}
void cleanup(void) {}