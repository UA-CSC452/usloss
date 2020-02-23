#include <stdio.h>
#include <assert.h>
#include "usloss.h"
#include "globals.h"

int ticks = 0;

void 
clock_handler(int type, void *arg) {
    static int last = 0;
    int now;
    int status = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, 0, &now);
    if (status != USLOSS_DEV_OK) {
        USLOSS_Halt(status);
    }
    USLOSS_Console("Tick %d: %dus\n", ticks, now - last);
    last = now;
    ticks++;
}

void
startup(int argc, char **argv)
{
    if (virtual_time) {
        USLOSS_Console("This test is meant to test the real-time support, make sure USLOSS is in real-time mode\n");
        USLOSS_Halt(1);
    }
    int start, stop; 
    int status;
    int average; 
    time_t raverage;
    USLOSS_IntVec[USLOSS_CLOCK_INT] = clock_handler;
    // Turn on interrupts.
    status = USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    if (status != USLOSS_ERR_OK) {
        USLOSS_Halt(status);
    }
    struct timeval rstart;
    gettimeofday(&rstart, 0);
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
    struct timeval rstop;
    gettimeofday(&rstop, 0);
    time_t elapsed = ((rstop.tv_sec - rstart.tv_sec) * 1000000) + (rstop.tv_usec - rstart.tv_usec);
    raverage = elapsed / 100;
    assert(stop - start > 0);
    assert(elapsed > 0);
    average = (stop - start) / 100;
    assert((average > 18000) && (average < 21000));
    assert((raverage > 18000) && (raverage < 21000));
    USLOSS_Console("Virtual: Start = %dus, stop = %dus, elapsed = %dus, average = %dus\n", start, stop, stop - start, average);
    USLOSS_Console("Real: elapsed = %luus, average = %luus\n", elapsed, raverage);
    
    USLOSS_Halt(0);
}

void
finish(int argc, char **argv) {
    USLOSS_Console("Done.\n");
}
void test_setup(int argc, char **argv) {}
void test_cleanup(int argc, char **argv) {}
