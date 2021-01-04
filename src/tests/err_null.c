/*
 * err_null.c
 *
 * Simple test to make sure that the new error code USLOSS_MMU_ERR_NULL works
 * properly. Tests PageTable mode
 */
#include <stdio.h>
#include <assert.h>
#include "usloss.h"
#include "globals.h"

#define ASSERT_EQ(a, b) if ((a) != (b)) { USLOSS_Console("ASSERT FAIL: (%d != %d)\n", a, b); assert(0); }

void startup(int argc, char **argv)
{
    int rc;

    rc = USLOSS_MmuInit(2, 2, 2, USLOSS_MMU_MODE_PAGETABLE);
    ASSERT_EQ(rc, USLOSS_MMU_OK);

    // MmuGetConfig
    rc = USLOSS_MmuGetConfig(NULL, NULL, NULL, NULL, NULL, NULL);
    ASSERT_EQ(rc, USLOSS_MMU_OK);

    // others
    rc = USLOSS_MmuGetAccess(0, NULL);
    ASSERT_EQ(rc, USLOSS_MMU_ERR_NULL);
    rc = USLOSS_MmuGetPageTable(NULL);
    ASSERT_EQ(rc, USLOSS_MMU_ERR_NULL);

    USLOSS_Console("All tests passed\n");
    USLOSS_Halt(0);
}

void finish(int argc, char **argv)
{
    USLOSS_Console("Done.\n");
}

void test_setup(int argc, char **argv) {}
void test_cleanup(int argc, char **argv) {}