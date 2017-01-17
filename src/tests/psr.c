#include <usloss.h>
#include <stdio.h>
#include <stdlib.h>

void
dummy(int device, void *arg)
{}

void
startup(int argc, char **argv)
{
    int status;
    USLOSS_IntVec[USLOSS_CLOCK_INT] = dummy;
    USLOSS_Console("0x%x ", USLOSS_PsrGet());
    status = USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    if (status != USLOSS_ERR_OK){
        USLOSS_Halt(status);
    }
    USLOSS_Console("0x%x ", USLOSS_PsrGet());
    status = USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_INT);
    if (status != USLOSS_ERR_OK){
        USLOSS_Halt(status);
    }
    USLOSS_Console("0x%x ", USLOSS_PsrGet());
    status = USLOSS_PsrSet((USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_MODE) | USLOSS_PSR_CURRENT_INT);
    if (status != USLOSS_ERR_OK){
        USLOSS_Halt(status);
    }
    USLOSS_Console("0x%x\n", USLOSS_PsrGet());
    // Can't halt because we are in user mode.
    exit(0);
}

void
finish(int argc, char **argv) {}
void test_setup(int argc, char **argv) {}
void test_cleanup(int argc, char **argv) {}