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
    int i;
    for (i = 0; i < 4; i++) {
        USLOSS_Console("Test0\n");
        USLOSS_ContextSwitch(&context0, &context1);
    }
    USLOSS_Halt(0);
}
void
Test1(void)
{
    int i;
    for (i = 0; i < 4; i++) {
        USLOSS_Console("Test1\n");
        USLOSS_ContextSwitch(&context1, &context0);
    }
    USLOSS_Halt(0);
}

void helper(void)
{
    USLOSS_ContextInit(&context0, stack0, sizeof(stack0), NULL, Test0);
    USLOSS_ContextInit(&context1, stack1, sizeof(stack1), NULL, Test1);
    USLOSS_ContextSwitch(&context2, &context0);
}

void
startup(int argc, char **argv)
{
    USLOSS_Console("Startup\n");
    helper();
}
void
finish(int argc, char **argv)
{
    USLOSS_Console("Finish\n");
}
void test_setup(int argc, char **argv) {}
void test_cleanup(int argc, char **argv) {}