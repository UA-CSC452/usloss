// Sets the PSR to the command-line option. Returns the status code from USLOSS_PsrSet.
#include <usloss.h>
#include <stdio.h>
#include <stdlib.h>

void
startup(int argc, char **argv)
{
    int status;
    int psr;
    
    if (argc != 2) {
        USLOSS_Console("Usage: %s psr\n", argv[0]);
        USLOSS_Halt(1);
    }
    USLOSS_Console("%s %s\n", argv[0], argv[1]);
    status = USLOSS_PsrSet(atoi(argv[1]));
    if (status != USLOSS_ERR_OK) {
        USLOSS_Halt(status);
    }
    psr = USLOSS_PsrGet();
    if (psr != atoi(argv[1])) {
        USLOSS_Halt(1);
    }
    USLOSS_Halt(0);
}

void
finish(int argc, char **argv) {}
void test_setup(int argc, char **argv) {}
void test_cleanup(int argc, char **argv) {}