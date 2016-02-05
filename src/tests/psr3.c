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
    USLOSS_Console("psr = %d\n", USLOSS_PsrGet());
    USLOSS_Console("This will cause a trap.\n");
    USLOSS_PsrSet(0);
}

void
finish() {}
