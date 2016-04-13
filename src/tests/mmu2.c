#include "usloss.h"
#include <stdio.h>
#include <stdlib.h>
#include "mmu.h"

USLOSS_Context   context0, context1;
char             stack[USLOSS_MIN_STACK];

/*
 * Test1
 *
 * Tests for fault, access bits set properly
 */
void
Test1(void)
{
    int         page;
    int         frame;
    int         dummy;

    USLOSS_Console("Test 1 starting\n");
}


void 
startup(void)
{
    USLOSS_Console("Running tests.\n");
    USLOSS_Console("Test1.\n");
    USLOSS_ContextInit(&context1, USLOSS_PsrGet(), stack, sizeof(stack), Test1);
    USLOSS_ContextSwitch(&context0, &context1);      

    USLOSS_Halt(0);
}

void
finish()
{
    return;
}
void setup(void) {}
void cleanup(void) {}