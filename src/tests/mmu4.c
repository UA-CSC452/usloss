/*
 * mmu4.c
 *
 * Tests page protections in page table mode.
 * To use this test compile it and link it against the USLOSS library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "usloss.h"

int wantPage = -1;
int wantCause = 0;
int pageSize;
int test;
char *segment;

#define NUMPAGES 2
USLOSS_PTE  pageTable[NUMPAGES];

#define PageAddr(i)     (segment + ((i) * pageSize))
#define PageIndex(offset) ((offset) / pageSize)

#define Expect(page, cause) wantPage = (page); wantCause = (cause)


static void
CheckPage(int page, int offset)
{
    int realPage;
    realPage = PageIndex(offset);
    if (realPage != page) {
       USLOSS_Console("Fault caused by wrong page: %d != %d\n", realPage, page);
       USLOSS_Halt(1);
    }
}
static void
CheckCause(int cause) 
{
    int realCause;

    realCause = USLOSS_MmuGetCause();
    if ((realCause & cause) == 0) {
       USLOSS_Console("Wrong cause: %d != %d\n", realCause, cause);
       USLOSS_Halt(1);
    }
}

void
Handler(int type, void *offsetArg)
{
    int     page = wantPage;
    int     cause = wantCause;
    int     offset = (int) offsetArg;

    if (type != USLOSS_MMU_INT) {
        return;
    }
    CheckPage(page, offset);
    (void) CheckCause(cause);
    USLOSS_Halt(0);
}

void
Test0(void)
{
    int status;
    // Map page 0 to frame 0, no access.

    pageTable[0].incore = 1;
    pageTable[0].frame = 0;

    status = USLOSS_MmuSetPageTable(pageTable);
    assert(status == 0);

    // Write 'A' to page 0 (frame 0). The page is not writeable so we should get
    // a protection violation.

    Expect(0, USLOSS_MMU_ACCESS);
    *segment = 'A';
}

void 
Test1(void)
{
    int status;
    // Map page 0 to frame 0, read-only.

    pageTable[0].incore = 1;
    pageTable[0].frame = 0;
    // Make page read-only, write should fail.
    pageTable[0].read = 1;

    status = USLOSS_MmuSetPageTable(pageTable);
    assert(status == 0);
    Expect(0, USLOSS_MMU_ACCESS);
    *segment = 'A';
}

void 
Test2(void)
{
    int status;
    // Map page 0 to frame 0, read/write.

    pageTable[0].incore = 1;
    pageTable[0].frame = 0;
    pageTable[0].read = 1;
    pageTable[0].write = 1;

    status = USLOSS_MmuSetPageTable(pageTable);
    assert(status == 0);
    Expect(0, 0);
    *segment = 'A';
    assert(*segment == 'A');
}


void 
Test3(void)
{
    int status;

    Test2();
    // Now make page inaccessible and try to read.

    pageTable[0].read = 0;
    pageTable[0].write = 0;

    status = USLOSS_MmuSetPageTable(pageTable);
    assert(status == 0);
    Expect(0, USLOSS_MMU_ACCESS);
    assert(*segment == 'A');
}

void 
Test4(void)
{
    int status;

    Test2();
    // Now make page read-only and try to write.

    pageTable[0].read = 1;
    pageTable[0].write = 0;

    status = USLOSS_MmuSetPageTable(pageTable);
    assert(status == 0);
    Expect(0, USLOSS_MMU_ACCESS);
    *segment = 'A';
}

void 
Test5(void)
{
    int status;
    char *addr;

    // Page fault

    status = USLOSS_MmuSetPageTable(pageTable);
    assert(status == 0);
    Expect(1, USLOSS_MMU_FAULT);
    addr = PageAddr(1);
    *addr = 'A';
}

void 
startup(int argc, char **argv)
{
    int		   status;
    int		   pages;
    int        i;

    test = atoi(argv[1]);
    for (i = 0; i < USLOSS_NUM_INTS; i++) {
        USLOSS_IntVec[i] = Handler;
    }
    for (i = 0; i < NUMPAGES; i++) {
        pageTable[i].incore = 0;
        pageTable[i].read = 0;
        pageTable[i].write = 0;
    }

    // 2 entries per page table, 2 pages, 2 page frames

    status = USLOSS_MmuInit(NUMPAGES, NUMPAGES, NUMPAGES, USLOSS_MMU_MODE_PAGETABLE);
    assert(status == USLOSS_MMU_OK);
    segment = USLOSS_MmuRegion(&pages);
    assert(segment != NULL);
    assert(pages == NUMPAGES);
    pageSize = USLOSS_MmuPageSize();

    switch(test) {
        case 0: Test0(); break;
        case 1: Test1(); break;
        case 2: Test2(); break;
        case 3: Test3(); break;
        case 4: Test4(); break;
        case 5: Test5(); break;
        default:
            USLOSS_Console("Invalid test %d\n", test);
            USLOSS_Halt(1);
    }
    USLOSS_Halt(0);
}

void
finish(int argc, char **argv)
{
    return;
}
void test_setup(int argc, char **argv) {}
void test_cleanup(int argc, char **argv) {}