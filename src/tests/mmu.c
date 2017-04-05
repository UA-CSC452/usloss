#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <setjmp.h>
#include "usloss.h"

#define NUMPAGES	4
#define NUMFRAMES	4
#define NUMMAPS		NUMPAGES

typedef struct PTE {
    int		protection;
    int		frame;
} PTE;

typedef struct State {
    int		maps;
    int		frames;
    int		pages;
    PTE		*pageTable[USLOSS_MMU_NUM_TAG];
} State;

char		*segment;
int		pageSize;
State		state;

#define PageAddr(i) 	(segment + ((i) * pageSize))
#define PageIndex(offset) ((offset) / pageSize)

#define Expect(page, cause) wantPage = (page); wantCause = (cause)

int	wantPage;
int	wantCause;

#define SWAP() { 						\
    int tmp = active;						\
    active = !active; 						\
    USLOSS_ContextSwitch(&contexts[tmp], &contexts[!tmp]);		\
}

#define LOAD(func) {						\
    USLOSS_ContextInit(&contexts[!active], stack, USLOSS_MIN_STACK, NULL, func);\
}

char			stack[USLOSS_MIN_STACK];
static USLOSS_Context	contexts[2];
static int		active = 0;
static int		tag;

void
dummy_handler(type, arg)
    int		type;
    void	*arg;
{
#ifdef NOTDEF
    fprintf(stderr, "Interrupt: %d\n", type);
#endif /* NOTDEF */
    return;
}
void
CheckPage(int page, int offset)
{
    int	realPage;
    realPage = PageIndex(offset);
    if (realPage != page) {
	   USLOSS_Console("Fault caused by wrong page: %d != %d\n", realPage, page);
	   abort();
    }
}
int
CheckCause(int cause) 
{
    int	realCause;

    realCause = USLOSS_MmuGetCause();
    if ((realCause & cause) == 0) {
	   USLOSS_Console("Wrong cause: %d != %d\n", realCause, cause);
	   abort();
    }
    return realCause;
}

void
Handler(int type, void *offsetArg)
{
    int		status;
    int		page = wantPage;
    int		cause = wantCause;
    int		frame;
    int		tmp;
    int		offset = (int) offsetArg;

    CheckPage(page, offset);
    cause = CheckCause(cause);
    if (cause == USLOSS_MMU_FAULT) { 
	status = USLOSS_MmuGetTag(&tmp);
	assert(status == USLOSS_MMU_OK);
	if (tmp == -1) {
	    status = USLOSS_MmuSetTag(tag);
	    return;
	}
	frame = state.pageTable[tag][page].frame;
	if (frame == -1) {
	    SWAP();
	}
	status = USLOSS_MmuMap(tag, page, frame, 
		    state.pageTable[tag][page].protection);
	assert(status == USLOSS_MMU_OK);
    } else {
	assert(cause == USLOSS_MMU_ACCESS);
	SWAP();
    }
}

void
CheckAccess(int frame, int access)
{
    int	tmp;
    int	status;

    status = USLOSS_MmuGetAccess(frame, &tmp);
    if (tmp != access) {
	USLOSS_Console("Invalid access bits for frame %d: %d != %d\n",
	    frame, tmp, access);
	abort();
    }
}

void
AddPTE(int page, int frame, int prot)
{
    state.pageTable[tag][page].frame = frame;
    state.pageTable[tag][page].protection = prot;
}

void
Unmap(int page)
{
    int	result;
    int	frame;

    frame = state.pageTable[tag][page].frame;
    assert(frame != -1);
    AddPTE(page, -1, 0);
    result = USLOSS_MmuUnmap(tag, page);
    assert(result == 0);
    result = USLOSS_MmuSetAccess(frame, 0);
    assert(result == 0);
}


void 
UnmapAll(void) 
{
    int		i;

    for (i = 0; i < state.pages; i++) {
	if (state.pageTable[tag][i].frame != -1) {
	    Unmap(i);
	}
    }
}
/*
 * Test1
 *
 * Tests for fault, access bits set properly
 */
void
Test1(void)
{
    int		page;
    int		frame;
    int		dummy;

    //USLOSS_Console("Test 1\n");
    assert(sizeof(USLOSS_PTE) == 4);
    page = 0;
    frame = page;
    AddPTE(page, frame, USLOSS_MMU_PROT_RW);
    CheckAccess(frame, 0);
    Expect(page, USLOSS_MMU_FAULT);
    dummy = *segment;
    CheckAccess(frame, USLOSS_MMU_REF);
    *segment = 'A';
    CheckAccess(frame, USLOSS_MMU_REF|USLOSS_MMU_DIRTY);
    if (*segment != 'A') {
	USLOSS_Console("Didn't read what I wrote\n");
	abort();
    }
}
/*
 * Test2
 *
 * Read access violation.
 */

void
Test2(void)
{
    int		page;
    int		frame;
    int		dummy;

    //USLOSS_Console("Test 2\n");
    page = 0;
    frame = page;
    AddPTE(page, frame, USLOSS_MMU_PROT_NONE);
    CheckAccess(frame, 0);
    Expect(page, USLOSS_MMU_FAULT|USLOSS_MMU_ACCESS);
    dummy = *segment;
    /*
     * Should get an access violation and not get here.
     */
    USLOSS_Console("Test2 survived access violation\n");
    abort();
}
/*
 * Test3
 *
 * Write access violation.
 */
void
Test3(void)
{
    int		page;
    int		frame;
    int		dummy;

    //USLOSS_Console("Test 3\n");
    page = 0;
    frame = page;
    AddPTE(page, frame, USLOSS_MMU_PROT_READ);
    CheckAccess(frame, 0);
    Expect(page, USLOSS_MMU_FAULT);
    dummy = *segment;
    CheckAccess(frame, USLOSS_MMU_REF);
    Expect(page, USLOSS_MMU_ACCESS);
    *segment = 'A';
    /*
     * Should get an access violation and not get here.
     */
    USLOSS_Console("Test3 survived access violation\n");
    abort();
}
/*
 * Test4
 *
 * Access bit manipulation.
 */
void
Test4(void)
{
    int		page;
    int		frame;
    int		dummy;
    int		status;

    //USLOSS_Console("Test 4\n");
    page = 0;
    frame = page;
    AddPTE(page, frame, USLOSS_MMU_PROT_RW);
    CheckAccess(frame, 0);
    /*
     * Make sure the reference bit gets turned on and cleared ok.
     */
    Expect(page, USLOSS_MMU_FAULT);
    dummy = *segment;
    CheckAccess(frame, USLOSS_MMU_REF);
    status = USLOSS_MmuSetAccess(frame, 0);
    assert(status == USLOSS_MMU_OK);
    CheckAccess(frame, 0);
    dummy = *segment;
    CheckAccess(frame, USLOSS_MMU_REF);
    status = USLOSS_MmuSetAccess(frame, 0);
    assert(status == USLOSS_MMU_OK);
    CheckAccess(frame, 0);
    /*
     * Make sure the dirty bit gets turned on and cleared ok.
     */
    *segment = 'A';
    CheckAccess(frame, USLOSS_MMU_REF|USLOSS_MMU_DIRTY);
    status = USLOSS_MmuSetAccess(frame, 0);
    assert(status == USLOSS_MMU_OK);
    CheckAccess(frame, 0);
    *segment = 'A';
    CheckAccess(frame, USLOSS_MMU_REF|USLOSS_MMU_DIRTY);
    status = USLOSS_MmuSetAccess(frame, 0);
    assert(status == USLOSS_MMU_OK);
    CheckAccess(frame, 0);
    dummy = *segment;
    CheckAccess(frame, USLOSS_MMU_REF);
}

/*
 * Test5
 *
 * Share a page frame.
 */
void
Test5(void)
{
    int		page;
    int		frame;
    char	*addr;
    static char	*msg = "Hello world"; 

    //USLOSS_Console("Test 5\n");
    page = 0;
    frame = page;
    AddPTE(page, frame, USLOSS_MMU_PROT_RW);
    AddPTE(page+1, frame, USLOSS_MMU_PROT_RW);
    addr = PageAddr(page);
    Expect(page, USLOSS_MMU_FAULT);
    strcpy(addr, msg);
    addr = PageAddr(page+1);
    Expect(page+1, USLOSS_MMU_FAULT);
    if (strcmp(addr, msg)) {
	   USLOSS_Console("Page sharing failed\n");
    }
}
/*
 * Test6
 *
 * Fault on unmapped page.
 */
void
Test6(void)
{
    int		page;
    int		frame;
    static char	*msg = "Hello world"; 

    //USLOSS_Console("Test 6\n");
    page = 0;
    frame = page;
    AddPTE(page, frame, USLOSS_MMU_PROT_RW);
    Expect(page, USLOSS_MMU_FAULT);
    strcpy(segment, msg);
    Unmap(page);
    Expect(page, USLOSS_MMU_FAULT);
    USLOSS_Console(segment);
    /*
     * Should get fault and not get here.
     */
    USLOSS_Console("Test6 survived fault\n");
    abort();
}
/*
 * Test7
 *
 * Test tag functionality
 */
void
Test7(void)
{
    int		page;
    int		frame;
    char	*addr;
    int		result;
    static char	*msgs[2] = {"Hello world", "Happy Birthday"}; 

    //USLOSS_Console("Test 7\n");
    page = 1;
    addr = PageAddr(page);

    for (tag = 0; tag < 2; tag++) {
	frame = tag;
	AddPTE(page, frame, USLOSS_MMU_PROT_RW);
	result = USLOSS_MmuSetTag(tag);
	assert(result == USLOSS_MMU_OK);
	Expect(page, USLOSS_MMU_FAULT);
	strcpy(addr, msgs[tag]);
    }

    for (tag = 0; tag < 2; tag++) {
	result = USLOSS_MmuSetTag(tag);
	assert(result == USLOSS_MMU_OK);
	Expect(page, USLOSS_MMU_FAULT);
	if (strcmp(addr, msgs[tag])) {
	    USLOSS_Console("Tag test failed\n");
	}
    }
}

/*
 * Test8
 *
 * Test that real segmentation violations dump core.
 */
void
Test8(void)
{
    //USLOSS_Console("Test 8\n");
    //USLOSS_Console("This should cause a segmentation violation.\n");
    *((volatile int *) NULL) = 0;
    USLOSS_Console("Test 8 still running?\n");
}



static void (*Tester)();

void Wrapper(void)
{
    (*Tester)();
    SWAP();
}

void
DoTest(void (*test)())
{
    Tester = test;
    LOAD(Wrapper);
    SWAP();
    UnmapAll();
}

void 
startup(int argc, char **argv)
{
    int		i;
    int		status;
    int		dummy;

    pageSize = USLOSS_MmuPageSize();
    //USLOSS_Console("Page size = %d\n", pageSize);
    for (i = 0; i < USLOSS_NUM_INTS; i++) {
	   USLOSS_IntVec[i] = dummy_handler;
    }
    USLOSS_IntVec[USLOSS_MMU_INT] = Handler;

    state.maps = NUMMAPS;
    state.pages = NUMPAGES;
    state.frames = NUMFRAMES;
    for (tag = 0; tag < USLOSS_MMU_NUM_TAG; tag++) {
	state.pageTable[tag] = (PTE *) malloc(state.pages * sizeof(PTE));
	for (i = 0; i < state.pages; i++) {
	    state.pageTable[tag][i].frame = -1;
	}
    }

    status = USLOSS_MmuInit(state.maps,state.pages,state.frames, USLOSS_MMU_MODE_TLB);
    assert(status == USLOSS_MMU_OK);
    segment = USLOSS_MmuRegion(&dummy);
    assert(segment != NULL);
    assert(dummy >= state.pages);

    tag = 0;

    status = USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    assert(status == USLOSS_ERR_OK);


    DoTest(Test1);
    DoTest(Test2);
    DoTest(Test3);
    DoTest(Test4);
    DoTest(Test5);
    DoTest(Test6);
    DoTest(Test7);
    //USLOSS_Console("All simple tests completed.\n");
    DoTest(Test8);
    USLOSS_Halt(0);
}

void
finish(int argc, char **argv)
{
    return;
}
void test_setup(int argc, char **argv) {}
void test_cleanup(int argc, char **argv) {}