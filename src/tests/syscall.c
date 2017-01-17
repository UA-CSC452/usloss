
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
startup(int argc, char **argv)
{
    int	 i;
    int status;

    USLOSS_IntVec[USLOSS_SYSCALL_INT] = syscall_handler;

    // Switch to user mode

    status = USLOSS_PsrSet((USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_MODE) | USLOSS_PSR_CURRENT_INT);
    if (status != USLOSS_ERR_OK) {
        USLOSS_Halt(status);
    }
    for (i = 0; i < 100; i++) {
	   USLOSS_Syscall((void *) i);
    }
    // Never gets here.

}

void
finish(int argc, char **argv) {
    USLOSS_Console("Done.\n");
}
void test_setup(int argc, char **argv) {}
void test_cleanup(int argc, char **argv) {}