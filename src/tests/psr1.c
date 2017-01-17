#include <usloss.h>

static void 
handler(int dev, void *arg) {
    USLOSS_Console("Protection Violation.\n");
    USLOSS_Halt(0);
}

void
startup(int argc, char **argv)
{
    int status;
    int psr;
    USLOSS_IntVec[USLOSS_ILLEGAL_INT] = handler;
    psr = (USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_MODE) | USLOSS_PSR_CURRENT_INT;
    status = USLOSS_PsrSet(psr);
    if (status != USLOSS_ERR_OK) {
        USLOSS_Halt(status);
    }
    if (psr != USLOSS_PsrGet()) {
        USLOSS_Halt(1);
    }
    USLOSS_Console("0x%x\n", USLOSS_PsrGet());
    USLOSS_Console("This will cause a trap.\n");
    status = USLOSS_PsrSet(3);
    // Should never get here.
    if (status != USLOSS_ERR_OK) {
        USLOSS_Halt(status);
    }
    USLOSS_Halt(1);
}

void
finish(int argc, char **argv) {}
void test_setup(int argc, char **argv) {}
void test_cleanup(int argc, char **argv) {}