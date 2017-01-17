// This is an example of how USLOSS interrupts work. It simply reads
// characters from the terminals.

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "usloss.h"

#define NUMCHARS 5  // Number of characters to read per terminal. Don't make this too big.

char inputs[USLOSS_TERM_UNITS][NUMCHARS];   // What we expect to read.
char buffers[USLOSS_TERM_UNITS][NUMCHARS];  // What we actually read.
int  counts[USLOSS_TERM_UNITS];             // # of characters in each buffer.
int done = 0;                               // # of terminals complete.

//
// Read characters from the terminals and puts them in the buffers.
// When a buffer is full verify its content against the inputs.
//
void 
term_handler(int type, void *arg) {
    int 	result;
    int 	status;
    char 	ch;
    int		unit = (int) arg;
    int     i;

    // Get the device status for the terminal.
    result = USLOSS_DeviceInput(USLOSS_TERM_DEV, unit, &status);
    if (result != USLOSS_DEV_OK) {
	   USLOSS_Console("Something is wrong with terminal %d!!", unit);
	   USLOSS_Halt(1);
    }
    // If the status is "USLOSS_DEV_BUSY" then a character has been received.
    if (USLOSS_TERM_STAT_RECV(status) == USLOSS_DEV_BUSY) {
	   ch = USLOSS_TERM_STAT_CHAR(status);	// Get the characte
	   USLOSS_Console("%d: %c\n", unit, ch);
       i = counts[unit]++;
       buffers[unit][i] = ch;
       if (i == NUMCHARS-1) {
            assert(!strncmp(inputs[unit], buffers[unit], NUMCHARS));
            done++;
        }
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
startup(int argc, char **argv)
{
    int	status;
    int i;

    for (i = 0; i < USLOSS_NUM_INTS; i++) {
	   USLOSS_IntVec[i] = dummy_handler;
    }
    // Turn on receive interrupts.

    for(i = 0; i < USLOSS_TERM_UNITS; i++) {
        status = USLOSS_DeviceOutput(USLOSS_TERM_DEV, i, (void *) USLOSS_TERM_CTRL_RECV_INT(0));
    }
    USLOSS_IntVec[USLOSS_TERM_INT] = term_handler;

    // Turn on interrupts.
    status = USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    assert(status == USLOSS_ERR_OK);

    // Read from the terminals.
    while(done < USLOSS_TERM_UNITS ) {
	   USLOSS_WaitInt();
    }
    USLOSS_Halt(0);
}

void
finish(int argc, char **argv) {}
void test_setup(int argc, char **argv) {
    int i,j,k;
    FILE    *f;
    char    name[50];
    int     n;

    // Compute the inputs and write them to the term*.in file. Each terminal reads unique content.
    k = 0;
    for(i = 0; i < USLOSS_TERM_UNITS; i++) {
        memset(buffers[i], '\0', NUMCHARS);
        counts[i] = 0;
        snprintf(name, sizeof(name), "term%d.in", i);
        f = fopen(name, "w");
        assert(f != NULL);
        for (j = 0; j < NUMCHARS; j++) {
            inputs[i][j] = 'a' + k++;
        }
        n = fwrite(inputs[i], 1, NUMCHARS, f);
        assert(n == NUMCHARS);
        fclose(f);
    }
}
void test_cleanup(int argc, char **argv) {}