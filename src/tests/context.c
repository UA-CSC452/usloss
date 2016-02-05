#include <stdio.h>
#include "usloss.h"

USLOSS_Context context0;
USLOSS_Context context1;
USLOSS_Context context2;


#define SIZE (USLOSS_MIN_STACK * 2)

char stack0[SIZE];
char stack1[SIZE];

void
Test0(void)
{
    while(1) {
        USLOSS_Console("Test0\n");
        USLOSS_ContextSwitch(&context0, &context1);
    }
}
void
Test1(void)
{
    while(1) {
        USLOSS_Console("Test1\n");
        USLOSS_ContextSwitch(&context1, &context0);
    }
}

void helper(void)
{
    USLOSS_ContextInit(&context0, USLOSS_PsrGet(), stack0, sizeof(stack0), Test0);
    USLOSS_ContextInit(&context1, USLOSS_PsrGet(), stack1, sizeof(stack1), Test1);
    USLOSS_ContextSwitch(&context2, &context0);
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
