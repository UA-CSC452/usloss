#include <stdio.h>
#include "usloss.h"

USLOSS_Context context0;
USLOSS_Context context1;

#define SIZE (USLOSS_MIN_STACK)

char stack0[SIZE];

void
Test0(void)
{
    USLOSS_Console("Test0\n");
}

void helper(void)
{
    USLOSS_ContextInit(&context0, USLOSS_PsrGet(), stack0, sizeof(stack0), Test0);
    USLOSS_ContextSwitch(&context1, &context0);
}

void
startup()
{
    USLOSS_Console("startup\n");
    helper();
}
void
finish(void)
{
    USLOSS_Console("Finishing\n");
}
void setup(void) {}
void cleanup(void) {}