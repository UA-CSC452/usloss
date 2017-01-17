/*
 * mmu3.c
 *
 * This is a simple test that demonstrates the use of the USLOSS MMU in page table mode. It creates
 * a VM region with 2 entries in the page table, 2 pages, and 2 frames. It then maps page 0 -> frame
 * 0 and writes 'A'. It then maps page 0 -> frame 1 and verifies that it reads '8' (the default
 * value with which page frames are filled). It then writes 'B' and maps page 0 -> frame 0 and
 * verifies that it still reads 'A'. Finally, it maps page 1 -> frame 0 and verifies that it reads
 * 'A' there too.
 *
 * To use this test compile it and link it against the USLOSS library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "usloss.h"


void 
startup(int argc, char **argv)
{
    int		   status;
    int		   pages;
    char        *segment;
    int         size;
    USLOSS_PTE  pageTable[2];
    int         i;

    for (i = 0; i < 2; i++) {
        pageTable[i].incore = 0;
        pageTable[i].read = 1;
        pageTable[i].write = 1;
    }

    // 2 entries per page table, 2 pages, 2 page frames

    status = USLOSS_MmuInit(2, 2, 2, USLOSS_MMU_MODE_PAGETABLE);
    assert(status == USLOSS_MMU_OK);
    segment = USLOSS_MmuRegion(&pages);
    assert(segment != NULL);
    assert(pages == 2);
    size = USLOSS_MmuPageSize();

    // Map page 0 to frame 0

    pageTable[0].incore = 1;
    pageTable[0].frame = 0;

    status = USLOSS_MmuSetPageTable(pageTable);
    assert(status == 0);

    // Write 'A' to page 0 (frame 0)

    *segment = 'A';
    assert(*segment == 'A');

    // Now map page 0 to frame 1 and write 'B'

    pageTable[0].frame = 1;
    status = USLOSS_MmuSetPageTable(pageTable);
    assert(status == 0);

    // Frames are initially filled with '8's for debugging purposes.
    assert(*segment == '8');
    *segment = 'B';
    assert(*segment == 'B');

    // Go back to frame 0 and read 'A'

    pageTable[0].frame = 0;
    status = USLOSS_MmuSetPageTable(pageTable);
    assert(status == 0);
    assert(*segment == 'A');

    // Map page 1 to frame 0 and read 'A' there too.

    pageTable[1].incore = 1;
    pageTable[1].frame = 0;
    status = USLOSS_MmuSetPageTable(pageTable);
    assert(status == 0);

    assert(*(segment + size) == 'A');

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