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
    USLOSS_Halt(0);
}

void helper(void)
{
    USLOSS_ContextInit(&context0, stack0, sizeof(stack0), NULL, Test0);
    USLOSS_ContextSwitch(&context1, &context0);
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