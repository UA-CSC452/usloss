#include <usloss.h>
#include <stdio.h>

void
dummy(int device, void *arg)
{}

void
startup()
{
    USLOSS_Console("startup\n");
    USLOSS_IntVec[USLOSS_CLOCK_INT] = dummy;
    USLOSS_Console("1 psr = %d\n", USLOSS_PsrGet());
    USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    USLOSS_Console("2 psr = %d\n", USLOSS_PsrGet());
    USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_INT);
    USLOSS_Console("3 psr = %d\n", USLOSS_PsrGet());
    USLOSS_PsrSet((USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_MODE) | USLOSS_PSR_CURRENT_INT);
    USLOSS_Console("4 psr = %d\n", USLOSS_PsrGet());
    USLOSS_Console("This will cause a trap.\n");
    USLOSS_PsrSet(3);
}

void
finish() {}
