#include <stdio.h>
#include "usloss.h"

USLOSS_Context contexts[2];
int pid;

#define SIZE (USLOSS_MIN_STACK * 2)

char stacks[2][SIZE];


void
dummy_handler(type, arg)
    int		type;
    void	*arg;
{
    return;
}

void 
syscall_handler(int type, void *arg) {
    pid = !pid;
    USLOSS_ContextSwitch(&contexts[!pid], &contexts[pid]);
}

void
Process(void)
{
    USLOSS_PsrSet((USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_MODE) | USLOSS_PSR_CURRENT_INT);
    while(1) {
	USLOSS_Trace("Test %d\n", pid);
	USLOSS_Syscall(NULL);
    }
}

void
startup()
{
    int i;

    USLOSS_IntVec[USLOSS_CLOCK_INT] = dummy_handler;
    USLOSS_IntVec[USLOSS_SYSCALL_INT] = syscall_handler;
    for (i = 0; i < 2; i++) {
	USLOSS_ContextInit(&contexts[i], USLOSS_PsrGet(), stacks[i], sizeof(stacks[i]), Process);
    }
    pid = 0;
    USLOSS_ContextSwitch(NULL, &contexts[0]);
}
void
finish(void)
{
    USLOSS_Console("Finishing\n");
}
void setup(void) {}
void cleanup(void) {}