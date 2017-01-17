// Two processes that make system calls.
#include <stdio.h>
#include <assert.h>
#include "usloss.h"

USLOSS_Context contexts[2];
int pid;

#define SIZE (USLOSS_MIN_STACK * 2)

char stacks[2][SIZE];
int count = 0;


void
dummy_handler(type, arg)
    int		type;
    void	*arg;
{
    return;
}

void 
syscall_handler(int type, void *arg) {
    if (count == 100) {
        USLOSS_Halt(0);
    }
    count++;
    pid = !pid;
    USLOSS_ContextSwitch(&contexts[!pid], &contexts[pid]);
}

void
Process(void)
{
    int status;
    status = USLOSS_PsrSet((USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_MODE) | USLOSS_PSR_CURRENT_INT);
    assert(status == USLOSS_ERR_OK);
    while(1) {
        USLOSS_Trace("Test %d\n", pid);
        USLOSS_Syscall(NULL);
    }
    // Never gets here.
}

void
startup(int argc, char **argv)
{
    int i;

    USLOSS_IntVec[USLOSS_CLOCK_INT] = dummy_handler;
    USLOSS_IntVec[USLOSS_SYSCALL_INT] = syscall_handler;
    for (i = 0; i < 2; i++) {
	   USLOSS_ContextInit(&contexts[i], stacks[i], sizeof(stacks[i]), NULL, Process);
    }
    pid = 0;
    USLOSS_ContextSwitch(NULL, &contexts[0]);
}
void
finish(int argc, char **argv)
{
    USLOSS_Console("Finishing\n");
}
void test_setup(int argc, char **argv) {}
void test_cleanup(int argc, char **argv) {}