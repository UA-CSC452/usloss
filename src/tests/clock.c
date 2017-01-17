
#include <stdio.h>
#include <assert.h>
#include "usloss.h"

int ticks = 0;

void 
clock_handler(int type, void *arg) {
    USLOSS_Console("Tick %d\n", ticks);
    ticks++;
}

void
startup(int argc, char **argv)
{
    int start, stop;
    int status;
    int average;
    USLOSS_IntVec[USLOSS_CLOCK_INT] = clock_handler;
    // Turn on interrupts.
    status = USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    if (status != USLOSS_ERR_OK) {
        USLOSS_Halt(status);
    }
    status = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, 0, &start);
    if (status != USLOSS_DEV_OK) {
        USLOSS_Halt(status);
    }
    while(ticks < 100) {
	   USLOSS_WaitInt();
    }
    status = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, 0, &stop);
    if (status != USLOSS_DEV_OK) {
        USLOSS_Halt(status);
    }
    assert(stop - start > 0);
    average = (stop - start) / 100;
    assert((average > 18000) && (average < 21000));
    USLOSS_Console("Start = %d, stop = %d, elapsed = %d, average = %d\n", start, stop, stop - start, (stop - start) / 100);
    USLOSS_Halt(0);
}

void
finish(int argc, char **argv) {
    USLOSS_Console("Done.\n");
}
void test_setup(int argc, char **argv) {}
void test_cleanup(int argc, char **argv) {}