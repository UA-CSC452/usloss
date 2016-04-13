#include <usloss.h>

void clock_handler(int dev, void *arg) {}

void
startup()
{
    USLOSS_IntVec[USLOSS_CLOCK_INT] = clock_handler;
    USLOSS_Console("psr = %d\n", USLOSS_PsrGet());
    USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    USLOSS_Console("psr = %d\n", USLOSS_PsrGet());
    USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_INT);
    USLOSS_Console("psr = %d\n", USLOSS_PsrGet());
    USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    USLOSS_Halt(0);
}

void
finish() {
    USLOSS_Console("finish\n");
    USLOSS_Console("psr = %d\n", USLOSS_PsrGet());
}
void setup(void) {}
void cleanup(void) {}