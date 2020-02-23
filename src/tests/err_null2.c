/*
 * err_null.c
 *
 * Simple test to make sure that the new error code USLOSS_MMU_ERR_NULL works
 * properly. Tests TLB Mode
 */
#include <stdio.h>
#include <assert.h>
#include "usloss.h"
#include "globals.h"

#define ASSERT_EQ(a, b) if ((a) != (b)) { USLOSS_Console("ASSERT FAIL: (%d != %d)\n", a, b); assert(0); }

void startup(int argc, char **argv)
{
    char *a = NULL;
    int rc;

    // TLB Mode
    rc = USLOSS_MmuInit(2, 2, 2, USLOSS_MMU_MODE_TLB);
    ASSERT_EQ(rc, USLOSS_MMU_OK);

    // map
    rc = USLOSS_MmuMap(0, 0, 0, 1);
    assert(rc == USLOSS_MMU_OK);
    rc = USLOSS_MmuGetMap(0, 0, NULL, (int*)a);
    ASSERT_EQ(rc, USLOSS_MMU_ERR_NULL);
    rc = USLOSS_MmuGetMap(0, 0, (int*)a, NULL);
    ASSERT_EQ(rc, USLOSS_MMU_ERR_NULL);
    rc = USLOSS_MmuGetTag(NULL);
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