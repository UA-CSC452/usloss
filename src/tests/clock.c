
#include <stdio.h>
#include "usloss.h"

int ticks = 0;

void 
clock_handler(int type, void *arg) {
    USLOSS_Console("Tick %d\n", ticks);
    ticks++;
}

void
startup()
{
    int start, stop;
    USLOSS_IntVec[USLOSS_CLOCK_INT] = clock_handler;
    // Turn on interrupts.
    USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    start = USLOSS_Clock();
    while(ticks < 100) {
	USLOSS_WaitInt();
    }
    stop = USLOSS_Clock();
    USLOSS_Console("Start = %d, stop = %d, elapsed = %d, average = %d\n", start, stop, stop - start, (stop - start) / 100);
    USLOSS_Halt(0);
}

void
finish() {
    USLOSS_Console("Done.\n");
}
