
#include <stdio.h>
#include "usloss.h"

int ticks = 0;

void 
syscall_handler(int type, void *arg) {
    USLOSS_Console("Syscall %d\n", (int) arg);
    if ((int) arg == 99) {
	USLOSS_Halt(0);
    }
}

void
startup()
{
    int	 i;

    USLOSS_IntVec[USLOSS_SYSCALL_INT] = syscall_handler;

    // Switch to user mode

    USLOSS_PsrSet((USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_MODE) | USLOSS_PSR_CURRENT_INT);
    for (i = 0; i < 100; i++) {
	USLOSS_Syscall((void *) i);
    }

}

void
finish() {
    USLOSS_Console("Done.\n");
}
void setup(void) {}
void cleanup(void) {}