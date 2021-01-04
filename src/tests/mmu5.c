/*
 * mmu5.c
 *
 * Simple test to test USLOSS_MmuGetConfig and to check that the OS has complete
 * access to physical memory without the need of mapping its pages.
 *
 * To use this test compile it and link it against the USLOSS library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "usloss.h"

#define NUM_PAGES 2
#define NUM_FRAMES 3
#define NUM_MAPS 2

void 
startup(int argc, char **argv)
{
    int     status;
    int     pages;
    char    *segment;
    char    *pmem;
    int     size;

    // this should fail as the MMU is not initialized
    status = USLOSS_MmuGetConfig(NULL, NULL, NULL, NULL, NULL, NULL);
    assert(status == USLOSS_MMU_ERR_OFF);

    status = USLOSS_MmuInit(NUM_MAPS, NUM_PAGES, NUM_FRAMES, USLOSS_MMU_MODE_TLB);
    assert(status == USLOSS_MMU_OK);
    status = USLOSS_MmuGetConfig((void **)&segment, (void **)&pmem, &size, &pages, NULL, NULL);
    assert(status == USLOSS_MMU_OK);
    assert(segment != NULL);
    assert(pmem != NULL);
    assert(pages == NUM_PAGES);

    //  check that initial val is 8, then test write
    int i;
    for (i = 0; i < (NUM_FRAMES * size); i++) {
        assert(pmem[i] = '8');
        pmem[i] = 'A';
        assert(pmem[i] == 'A');
    }


    //USLOSS_Console("All tests passed.\n");
    USLOSS_Halt(0);
}

void
finish(int argc, char **argv)
{
    return;
}
void test_setup(int argc, char **argv) {}
void test_cleanup(int argc, char **argv) {}
