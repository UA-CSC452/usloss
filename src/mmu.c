/*
 * mmu.c
 *
 *      MMU implementation for usloss. See the documentation for
 *      more details. Basically, the MMU supports mappings from virtual
 *      pages to physical page frames. This is done using mmap and
 *      mprotect. You can configure it so that there are fewer mappings
 *      than virtual pages, (i.e. a TB), and you can associate a tag
 *      with each mapping. Each mapping can also be read-only or read-write.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include "usloss.h"
#include "globals.h"
#include <setjmp.h>
#include <fcntl.h>

extern void set_timer(void);

/*
 * Per-page information. The virtProt and realProt are used to implement
 * different page protections, plus the reference and dirty bits on the
 * frames. virtProt is the protection on the virtual page, so that if
 * an invalid access is done the USLOSS_MMU_INT handler is called with a cause
 * of USLOSS_MMU_ACCESS. realProt is the real protection on the page, which is
 * used to both enforce the virtProt and to set the access bits.
 */
typedef struct MMUPage {
    int         frame;          /* Frame that contains page */
    int         virtProt;       /* Protection on virtual page */
    int         realProt;       /* Protection of real page. */
} MMUPage;

/*
 * Per-frame information.
 */
typedef struct MMUFrame {
    int         access;         /* Access bits for frame */
} MMUFrame;


/*
 * Global info for the MMU.
 */
typedef struct MMUInfo {
    int         fd;             /* FD for physical memory */
    int         numFrames;      
    MMUFrame    *frames;
    int         numPages;
    MMUPage     *pages[USLOSS_MMU_NUM_TAG];
    int         maxMaps;        /* max # valid mappings */
    int         numMaps;        /* current # of mappings */
    int         cause;          /* Cause of the last MMU exception */
    void        *region;        /* aligned vm region */
    int         tag;            /* Current tag */
    USLOSS_PTE  *pageTable;     /* Page table, if there is one. */
    int         mode;           /* PAGETABLE or TLB */
} MMUInfo;

static MMUInfo *mmuPtr = NULL;

#ifndef DEBUG
static int debugging = 0;
#else
static int debugging = 1;
#endif

#define PageAddr(i)     (mmuPtr->region + ((i) * mmuPageSize))
#define PageIndex(addr) (mmuPtr != NULL) ? \
    (((void *) (addr) - mmuPtr->region) / mmuPageSize) : 0;

typedef int Boolean;
#define TRUE 1
#define FALSE 0

static int      mmuPageSize;
Boolean  mmuInTouch = FALSE;
sigjmp_buf  mmuTouchBuf;
static int      nowhere;

static void SetRealProt(int page, int prot);
static int SetTag(int tag);

/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuInit --
 *
 *      Initialize the MMU by creating the physical memory file, 
 *      allocating the virtual memory region, and initializing
 *      the TB.
 *
 * Results:
 *      MMU return status
 *
 * Side effects:
 *      Memory is allocated and a file is mapped.
 *
 *----------------------------------------------------------------------
 */

int
USLOSS_MmuInit(numMaps, numPages, numFrames, mode)
    int         numMaps;        /* # of mappings. */
    int         numPages;       /* # of pages in the vm region. */
    int         numFrames;      /* # of page frames. */
    int         mode;           /* USLOSS_MMU_PAGETABLE or USLOSS_MMU_TLB */
{
    int                 fd = -1;
    FILE                *stream;
    int                 i;
    void                *region = NULL;
    int                 result;
    int                 tag;
    MMUPage             *pagePtr;
    int                 totalPages;
    char                *buffer;

    check_kernel_mode("USLOSS_MmuInit");
    debug("USLOSS_MmuInit: %d pages %d frames\n", numPages, numFrames);
    mmuPageSize = USLOSS_MmuPageSize();
    if (mmuPtr != NULL) {
        return USLOSS_MMU_ERR_ON;
    }
    if (numPages < 1) {
        return USLOSS_MMU_ERR_PAGE;
    }
    if (numFrames < 1) {
        return USLOSS_MMU_ERR_FRAME;
    }
    if ((numMaps < 1) || (numMaps > (numPages * USLOSS_MMU_NUM_TAG))) {
        return USLOSS_MMU_ERR_MAPS;
    }
    switch (mode) {
        case USLOSS_MMU_MODE_PAGETABLE:
        case USLOSS_MMU_MODE_TLB:
            break;
        default:
            return USLOSS_MMU_ERR_MODE;
    }
    stream = tmpfile();
    assert(stream != NULL);
    fd = fileno(stream);
    /*
     * Allocate the virtual region, plus a couple of guard pages.
     */
    totalPages = numPages+2;
    result = posix_memalign(&region, mmuPageSize, totalPages * mmuPageSize);
    assert(result == 0);
    memset(region, '7', totalPages * mmuPageSize);
    assert(USLOSS_MmuTouch(region) == TRUE);
    buffer = malloc(mmuPageSize);
    memset(buffer, '8', mmuPageSize);
    for (i = 0; i < numFrames + 1; i++) {
        write(fd, buffer, mmuPageSize);
    }
    free(buffer);
    nowhere = numFrames * mmuPageSize;
    debug("USLOSS_MmuInit: totalPages %d, nowhere 0x%x, file 0x%x\n",
        totalPages, nowhere, lseek(fd, 0, SEEK_CUR));
    result = mprotect(region, totalPages * mmuPageSize, PROT_NONE);
    assert(result == 0);
    assert(USLOSS_MmuTouch(region) == FALSE);
    region += mmuPageSize;

    mmuPtr = (MMUInfo *) malloc(sizeof(MMUInfo));
    mmuPtr->fd = fd;
    mmuPtr->numPages = numPages;
    mmuPtr->numFrames = numFrames;
    mmuPtr->maxMaps = numMaps;
    mmuPtr->numMaps = 0;
    mmuPtr->region = region;
    mmuPtr->cause = 0;
    mmuPtr->tag = 0;
    mmuPtr->mode = mode;
    /*
     * Allocate the page and frame information. Also unmap the region.
     */
    mmuPtr->frames = (MMUFrame *) malloc(numFrames * sizeof(MMUFrame));
    for (tag = 0; tag < USLOSS_MMU_NUM_TAG; tag++) {
        mmuPtr->pages[tag] = (MMUPage *) malloc(numPages * sizeof(MMUPage));
        for (i = 0; i < numPages; i++) {
            pagePtr = &mmuPtr->pages[tag][i];
            pagePtr->frame = -1;
            pagePtr->realProt = 0;
            pagePtr->virtProt = 0;
        }
    }           
    for (i = 0; i < numFrames; i++) {
        mmuPtr->frames[i].access = 0;
    }
    return USLOSS_MMU_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuRegion --
 *
 *      Returns a pointer to the VM region
 *
 * Results:
 *      Pointer to the VM region.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
void *
USLOSS_MmuRegion(numPagesPtr)
    int         *numPagesPtr;   /* # pages in region */
{
    if (mmuPtr == NULL) {
        *numPagesPtr = 0;
        return NULL;
    }
    *numPagesPtr = mmuPtr->numPages;
    debug("USLOSS_MmuRegion: 0x%p %d pages\n", mmuPtr->region, mmuPtr->numPages);
    return mmuPtr->region;
}

/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuDone --
 *
 *      Cleans up a vm region.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
int
USLOSS_MmuDone()
{
    int         result;
    int         i;

    check_kernel_mode("USLOSS_MmuDone");
    if (mmuPtr == NULL) {
        return USLOSS_MMU_ERR_OFF;
    }
    debug("USLOSS_MmuDone: unmapping 0x%p, %d bytes\n", mmuPtr->region,
        mmuPtr->numPages * mmuPageSize);
    result = mprotect(mmuPtr->region, mmuPtr->numPages * mmuPageSize,
                PROT_NONE);
    if (result != 0) {
        perror("USLOSS_MmuDone: mprotect");
        abort();
    }
    for (i = 0; i < USLOSS_MMU_NUM_TAG; i++) {
        free((char *) mmuPtr->pages[i]);
    }
    free((char *) mmuPtr->frames);
    free((char *) mmuPtr);
    mmuPtr = NULL;
    return USLOSS_MMU_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Map --
 *
 *      Internal function that maps a page to the given frame.
 *
 * Results:
 *      MMU return status.
 *
 * Side effects:
 *      Memory is mapped.
 *
 *----------------------------------------------------------------------
 */
static int
Map(tag, page, frame, protection)
    int         tag;            /* tag associated with map. */
    int         page;           /* Page to be mapped. */
    int         frame;          /* Frame to map the page into. */
    int         protection;     /* Protection for the frame. */
{
    void        *addr;
    MMUPage     *pagePtr;

    if ((page < 0) || (page >= mmuPtr->numPages)) {
        return USLOSS_MMU_ERR_PAGE;
    }
    if ((frame < 0) || (frame >= mmuPtr->numFrames)) {
        return USLOSS_MMU_ERR_FRAME;
    }
    if (mmuPtr->numMaps == mmuPtr->maxMaps) {
        return USLOSS_MMU_ERR_MAPS;
    }
    if ((protection & (~(USLOSS_MMU_PROT_RW))) != 0) {
        return USLOSS_MMU_ERR_PROT;
    }
    if ((tag < 0) || (tag >= USLOSS_MMU_NUM_TAG)) {
        return USLOSS_MMU_ERR_TAG;
    }
    pagePtr = &mmuPtr->pages[tag][page];
    if (pagePtr->frame != -1) {
        return USLOSS_MMU_ERR_REMAP;
    }
    if (tag == mmuPtr->tag) {
        debug("Map: mmap 0x%p -> 0x%x\n", PageAddr(page),
           frame * mmuPageSize);
        (void) msync(PageAddr(page), mmuPageSize, MS_SYNC);
        (void) munmap(PageAddr(page), mmuPageSize);
        addr = mmap(PageAddr(page), mmuPageSize, PROT_NONE, 
                    MAP_SHARED|MAP_FIXED, mmuPtr->fd, frame * mmuPageSize);
        assert(addr != MAP_FAILED);
        assert(addr == PageAddr(page));
    }
    debug("Map: mapping page %d (0x%p) -> %d\n", page, PageAddr(page),
        frame);
    mmuPtr->numMaps++;
    assert(mmuPtr->numMaps <= mmuPtr->maxMaps);
    pagePtr->frame = frame;
    pagePtr->realProt = PROT_NONE;
    pagePtr->virtProt = protection;
    return USLOSS_MMU_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuMap --
 *
 *      Maps a page to the given frame.
 *
 * Results:
 *      MMU return status.
 *
 * Side effects:
 *      Memory is mapped.
 *
 *----------------------------------------------------------------------
 */
int
USLOSS_MmuMap(tag, page, frame, protection)
    int         tag;            /* tag associated with map. */
    int         page;           /* Page to be mapped. */
    int         frame;          /* Frame to map the page into. */
    int         protection;     /* Protection for the frame. */
{
    int         status;

    check_kernel_mode("USLOSS_MmuMap");
    if (mmuPtr == NULL) {
        return USLOSS_MMU_ERR_OFF;
    }
    if (mmuPtr->mode != USLOSS_MMU_MODE_TLB) {
        return USLOSS_MMU_ERR_MODE;
    }
    status = Map(tag, page, frame, protection);
    return status;
}

/*
 *----------------------------------------------------------------------
 *
 * Unmap --
 *
 *      Internal function that unmaps a page. The page must already be mapped.
 *
 * Results:
 *      MMU return status.
 *
 * Side effects:
 *      Memory is unmapped.
 *
 *----------------------------------------------------------------------
 */
static int
Unmap(tag, page)
    int         tag;    /* tag associated with page. */
    int         page;   /* Page to unmap. */
{
    MMUPage     *pagePtr;

    if ((page < 0) || (page >= mmuPtr->numPages)) {
        return USLOSS_MMU_ERR_PAGE;
    }
    if ((tag < 0) || (tag >= USLOSS_MMU_NUM_TAG)) {
        return USLOSS_MMU_ERR_TAG;
    }
    pagePtr = &mmuPtr->pages[tag][page];
    if (pagePtr->frame == -1) {
        return USLOSS_MMU_ERR_NOMAP;
    }
    debug("Unmap: unmapping page %d (0x%p)\n", page, PageAddr(page));
    if (tag == mmuPtr->tag) {
        void *addr;
        debug("Unmap: frame %d, virtProt %d, realProt %d\n", 
            pagePtr->frame, pagePtr->virtProt, pagePtr->realProt);
        (void) msync(PageAddr(page), mmuPageSize, MS_SYNC);
        (void) munmap(PageAddr(page), mmuPageSize);
        addr = mmap(PageAddr(page), mmuPageSize, PROT_NONE, 
                    MAP_SHARED|MAP_FIXED, mmuPtr->fd, nowhere);
        assert(addr != MAP_FAILED);
        assert(USLOSS_MmuTouch(PageAddr(page)) == FALSE);
    }
    mmuPtr->numMaps--;
    pagePtr->frame = -1;
    pagePtr->realProt = PROT_NONE;
    pagePtr->virtProt = 0;
    return USLOSS_MMU_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuUnmap --
 *
 *      Unmaps a page. The page must already be mapped.
 *
 * Results:
 *      MMU return status.
 *
 * Side effects:
 *      Memory is unmapped.
 *
 *----------------------------------------------------------------------
 */
int
USLOSS_MmuUnmap(tag, page)
    int         tag;    /* tag associated with page. */
    int         page;   /* Page to unmap. */
{
    int         status;

    check_kernel_mode("USLOSS_MmuUnmap");
    if (mmuPtr == NULL) {
        return USLOSS_MMU_ERR_OFF;
    }
    if (mmuPtr->mode != USLOSS_MMU_MODE_TLB) {
        return USLOSS_MMU_ERR_MODE;
    }
    status = Unmap(tag, page);
    return status;
}

/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuGetMap --
 *
 *      Returns the mapping for the given page under the given tag.
 *
 * Results:
 *      MMU return status.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
int
USLOSS_MmuGetMap(tag, page, framePtr, protPtr)
    int         tag;            /* tag for mapping */
    int         page;           /* Page for mapping */
    int         *framePtr;      /* Place to store frame */
    int         *protPtr;       /* Place to store protection */
{
    MMUPage     *pagePtr;

#ifdef NOTDEF
    check_kernel_mode("USLOSS_MmuGetMap");
#endif
    if (mmuPtr == NULL) {
        return USLOSS_MMU_ERR_OFF;
    }
    if (mmuPtr->mode != USLOSS_MMU_MODE_TLB) {
        return USLOSS_MMU_ERR_MODE;
    }
    if ((page < 0) || (page >= mmuPtr->numPages)) {
        return USLOSS_MMU_ERR_PAGE;
    }
    if ((tag < 0) || (tag >= USLOSS_MMU_NUM_TAG)) {
        return USLOSS_MMU_ERR_TAG;
    }
    pagePtr = &mmuPtr->pages[tag][page];
    if (pagePtr->frame == -1) {
        return USLOSS_MMU_ERR_NOMAP;
    }
    *framePtr = pagePtr->frame;
    *protPtr = pagePtr->virtProt;
    return USLOSS_MMU_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuGetCause --
 *
 *      Returns information about the last page fault or access violation.
 *
 * Results:
 *      0, USLOSS_MMU_FAULT, or USLOSS_MMU_ACCESS
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
int
USLOSS_MmuGetCause(void)
{
    int         cause = 0;

    check_kernel_mode("USLOSS_MmuGetCause");
    if (mmuPtr != NULL) {
        cause = mmuPtr->cause;
    }
    return cause;
}
        

/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuSetAccess --
 *
 *      Sets the frame access bits. Sets the reference bit if 
 *      USLOSS_MMU_REF is set and sets the dirty bit if USLOSS_MMU_DIRTY is set;
 *      otherwise the bits are cleared.
 *
 * Results:
 *      MMU return status.
 *
 * Side effects:
 *      The frame access bits are changed.
 *
 *----------------------------------------------------------------------
 */
int
USLOSS_MmuSetAccess(frame, access)
    int         frame;          /* Frame whose bits are to be modified. */
    int         access;         /* Access bits to be cleared. */
{
    int         i;
    int         prot;
    int         old;

    check_kernel_mode("USLOSS_MmuSetAccess");
    debug("USLOSS_MmuSetAccess: frame %d access %d\n", frame, access);
    if (mmuPtr == NULL) {
        return USLOSS_MMU_ERR_OFF;
    }
    if ((frame < 0) || (frame >= mmuPtr->numFrames)) {
        return USLOSS_MMU_ERR_FRAME;
    }
    if ((access & (~(USLOSS_MMU_REF|USLOSS_MMU_DIRTY))) != 0) {
        return USLOSS_MMU_ERR_ACC;
    }
    old = mmuPtr->frames[frame].access;
    mmuPtr->frames[frame].access = access;
    debug("USLOSS_MmuSetAccess: frame %d was %d is %d\n", frame,
        old, mmuPtr->frames[frame].access);
    /*
     * Now run through the page table and protect all pages mapped
     * to this frame so the access bits will be set properly.
     */
    if ((access & USLOSS_MMU_REF) == 0) {
        prot = PROT_NONE;
    } else if ((access & USLOSS_MMU_DIRTY) == 0) {
        prot = PROT_READ;
    } else {
        return USLOSS_MMU_OK;
    }
    for (i = 0; i < mmuPtr->numPages; i++) {
        if (mmuPtr->pages[mmuPtr->tag][i].frame == frame) {
            SetRealProt(i, prot);
        }
    }
    return USLOSS_MMU_OK;
}
/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuGetAccess --
 *
 *              Returns the access bits for the given frame.
 *
 * Results:
 *      MMU return status.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
int
USLOSS_MmuGetAccess(frame, accessPtr)
    int         frame;          /* Frame whose access bits are wanted.*/
    int         *accessPtr;     /* Pointer to the access bits. */
{
        
    check_kernel_mode("USLOSS_MmuGetAccess");
    if (mmuPtr == NULL) {
        return USLOSS_MMU_ERR_OFF;
    }
    if ((frame < 0) || (frame >= mmuPtr->numFrames)) {
        return USLOSS_MMU_ERR_FRAME;
    }
    *accessPtr = mmuPtr->frames[frame].access;
    return USLOSS_MMU_OK;
}
#define PROTS(real, virt) (((real) << 16) | (virt))
#define PROT_RW (PROT_READ|PROT_WRITE)

/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuHandler --
 *
 *      Called when a segmentation violation occurs. This routine determines
 *      if it's a page fault, access violation, a change in the access
 *      bits, or a real segmentation violation.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Unmaps the frame
 *
 *----------------------------------------------------------------------
 */
void
USLOSS_MmuHandler(sig, siginfoPtr, contextPtr)
    int         sig;            /* Which signal happened. */
    siginfo_t   *siginfoPtr;    /* Signal information. */
    ucontext_t  *contextPtr;    /* Unused. */

{
    int         page;
    int         interrupt = 0;
    MMUPage     *pagePtr;
    int         frame;
    int         old_psr = current_psr;
    int         result;

    assert(siginfoPtr != NULL);
    assert(sig == SIGSEGV || sig == SIGBUS);
    debug("USLOSS_MmuHandler: address 0x%p, psr 0x%x\n", siginfoPtr->si_addr,
        current_psr);
    page = PageIndex(siginfoPtr->si_addr);
    if (mmuInTouch) {
        debug("USLOSS_MmuHandler: address 0x%p touched\n", siginfoPtr->si_addr);
        goto done;
    }
    /*
     * If the MMU isn't initialized, or the page is invalid, let the
     * default handler handle it.
     */
    if ((mmuPtr == NULL) || (page < 0) || (page >= mmuPtr->numPages)) {
        result = sigaction(SIGSEGV, &old_actions[SIGSEGV], NULL);
        usloss_sys_assert(result != -1, "error setting SIGSEGV action");
        result = sigaction(SIGBUS, &old_actions[SIGBUS], NULL);
        usloss_sys_assert(result != -1, "error setting SIGBUS action");
        debug("USLOSS_MmuHandler: real segv (0x%p, %d)!!\n", mmuPtr, page);
        goto done;
    }
    /*
     * If the TAG is -1 or the page isn't mapped (frame is -1) 
     * then it's an MMU fault.
     */
    if ((mmuPtr->tag == -1) || 
        (mmuPtr->pages[mmuPtr->tag][page].frame == -1)) {
        mmuPtr->cause = USLOSS_MMU_FAULT;
        interrupt = 1;
        goto done;
    }
    pagePtr = &mmuPtr->pages[mmuPtr->tag][page];
    frame = pagePtr->frame;
    assert((pagePtr->realProt & (~PROT_RW)) == 0);
    assert((pagePtr->virtProt & (~USLOSS_MMU_PROT_RW)) == 0);
    /*
     * Access violations are harder to handle. What we do depends on the
     * virtual protection and real protection of the page. There are nine
     * different combintations: three are access violations, three require
     * the access bits to be updated, and three can't happen.
     */
    debug("USLOSS_MmuHandler: <%d,%d>\n", pagePtr->realProt, pagePtr->virtProt);
    switch(PROTS(pagePtr->realProt, pagePtr->virtProt)) {
        case PROTS(PROT_NONE,   USLOSS_MMU_PROT_NONE):
        case PROTS(PROT_READ,   USLOSS_MMU_PROT_READ):
        case PROTS(PROT_RW,     USLOSS_MMU_PROT_RW):
            debug("USLOSS_MmuHandler: access violation\n");
            mmuPtr->cause = USLOSS_MMU_ACCESS;
            interrupt = 1;
            break;
        case PROTS(PROT_READ,   USLOSS_MMU_PROT_RW):
            debug("USLOSS_MmuHandler: setting dirty bit\n");
            SetRealProt(page, PROT_RW);
            mmuPtr->frames[frame].access |= USLOSS_MMU_DIRTY;
            break;
        case PROTS(PROT_NONE,   USLOSS_MMU_PROT_READ):
        case PROTS(PROT_NONE,   USLOSS_MMU_PROT_RW):
            debug("USLOSS_MmuHandler: setting ref bit\n");
            SetRealProt(page, PROT_READ);
            mmuPtr->frames[frame].access |= USLOSS_MMU_REF;
            break;
        case PROTS(PROT_READ,   USLOSS_MMU_PROT_NONE):
        case PROTS(PROT_RW,     USLOSS_MMU_PROT_NONE):
        case PROTS(PROT_RW,     USLOSS_MMU_PROT_READ):
        default:
            fprintf(stderr, 
                "USLOSS: Internal error in USLOSS_MmuHandler\n");
            abort();
            break;
    }
done:
    if (mmuPtr != NULL) {
        debug("USLOSS_MmuHandler: addr 0x%p, cause %d\n", siginfoPtr->si_addr, 
            mmuPtr->cause);
        if (interrupt) {
            if (USLOSS_IntVec[USLOSS_MMU_INT] == NULL) {
                rpt_sim_trap("USLOSS_IntVec[USLOSS_MMU_INT] is NULL!\n");
            }
            (*USLOSS_IntVec[USLOSS_MMU_INT])(USLOSS_MMU_INT, 
                (void *) (siginfoPtr->si_addr - mmuPtr->region));
        }
        set_timer();
    }
    current_psr = old_psr;
}
/*
 *----------------------------------------------------------------------
 *
 * debug
 *
 *      Prints debugging output to the USLOSS_Console if the debugging flag
 *      is set.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
void debug(char *fmt, ...)
{
    va_list ap;

    if (debugging) {
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        fflush(stderr);
        va_end(ap);
    }
}
/*
 *----------------------------------------------------------------------
 *
 * SetRealProt
 *
 *      Sets the real protection on a page and updates the page information
 *      accordingly.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

static void
SetRealProt(page, prot)
    int         page;           /* Page to change. */
    int         prot;           /* New protection for page. */
{
    int         i;
    MMUPage     *pagePtr;
    void        *addr;

    pagePtr = &mmuPtr->pages[mmuPtr->tag][page];
    assert(pagePtr->frame != -1);
    debug("SetRealProt:  page %d (0x%p) real prot was %d is %d\n", page, 
        PageAddr(page), pagePtr->realProt, prot);
    pagePtr->realProt = prot;
    addr = mmap(PageAddr(page), mmuPageSize, prot, 
            MAP_SHARED|MAP_FIXED, mmuPtr->fd, 
            pagePtr->frame * mmuPageSize);
    assert(addr != MAP_FAILED);
    assert(addr == PageAddr(page));
    debug("SetRealProt: 0x%x -> 0x%x (0x%x)\n", PageAddr(page),
        pagePtr->frame * mmuPageSize, prot);
    for (i = 0; i < mmuPtr->numPages; i++) {
        if (mmuPtr->pages[mmuPtr->tag][i].frame == -1) {
            assert(USLOSS_MmuTouch(PageAddr(i)) == FALSE);
        }
    }
}
/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuSetTag
 *
 *      Sets the current tag in the MMU. SetTag does all the work.
 *
 * Results:
 *      MMU return status
 *
 * Side effects:
 *      See SetTag.
 *
 *----------------------------------------------------------------------
 */
int
USLOSS_MmuSetTag(new)
    int         new;    /* New tag */
{
    int         status;

    check_kernel_mode("USLOSS_MmuSetTag");
    if (mmuPtr == NULL) {
        return USLOSS_MMU_ERR_OFF;
    }
    if (mmuPtr->mode != USLOSS_MMU_MODE_TLB) {
        return USLOSS_MMU_ERR_MODE;
    }
    if ((new < 0) || (new >= USLOSS_MMU_NUM_TAG)) {
        return USLOSS_MMU_ERR_TAG;
    }
    status = SetTag(new);
    return status;
}
/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuGetTag
 *
 *      Gets the current tag in the MMU. 
 *
 * Results:
 *      MMU return status.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
int
USLOSS_MmuGetTag(tagPtr)
    int         *tagPtr;        /* Place to store tag */
{
    check_kernel_mode("USLOSS_MmuGetTag");
    if (mmuPtr == NULL) {
        return USLOSS_MMU_ERR_OFF;
    }
    if (mmuPtr->mode != USLOSS_MMU_MODE_TLB) {
        return USLOSS_MMU_ERR_MODE;
    }
    *tagPtr = mmuPtr->tag;
    return USLOSS_MMU_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * SetTag
 *
 *      Sets the current tag in the MMU. If the tag has changed then
 *      all pages associated with the old tag are unmapped, and all
 *      pages associated with the new tag are mapped.
 *
 *      This routine is not intended to be called outside of usloss.
 *      See USLOSS_MmuSetTag for an external routine.
 *
 * Results:
 *      MMU return status
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
static int
SetTag(new)
    int         new;    /* New tag */
{
    int         old;
    int         page;
    char        *addr;

    /*
     * Note that new can be -1, which is what usloss will set it to
     * on a context switch. This allows the vm system to detect a
     * context switch and remap accordingly, without requiring
     * modifications to earlier phases of the project.
     */

    check_kernel_mode("SetTag");
    debug("SetTag: %d\n", new);
    if ((new < -1) || (new >= USLOSS_MMU_NUM_TAG)) {
        return USLOSS_MMU_ERR_TAG;
    }
    if (mmuPtr == NULL) {
        return USLOSS_MMU_ERR_OFF;
    }
    old = mmuPtr->tag;
    if (old == new) {
        return USLOSS_MMU_OK;
    }
    if (old != -1) {
        for (page = 0; page < mmuPtr->numPages; page++) {
            if (mmuPtr->pages[old][page].frame != -1) {
                SetRealProt(page, PROT_NONE);
            }
        }
    }
    mmuPtr->tag = new;
    if (new != -1) {
        for (page = 0; page < mmuPtr->numPages; page++) {
            if (mmuPtr->pages[new][page].frame != -1) {
                addr = mmap(PageAddr(page), mmuPageSize, PROT_NONE, 
                        MAP_SHARED|MAP_FIXED, mmuPtr->fd, 
                        mmuPtr->pages[new][page].frame * mmuPageSize);
                assert(addr != MAP_FAILED);
                assert(addr == PageAddr(page));
            }
        }
    }
    return USLOSS_MMU_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuPageSize
 *
 *      Returns the page size.
 *
 * Results:
 *      Number of bytes in a page.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

int
USLOSS_MmuPageSize(void)
{
    return sysconf(_SC_PAGESIZE);
}


/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuTouch
 *
 *      Test accessibility of an address.
 *
 * Results:
 *      TRUE if address is accessible, FALSE otherwise.
 *
 * Side effects:
 *      May cause a segmentation violation
 *
 *----------------------------------------------------------------------
 */

Boolean
USLOSS_MmuTouch(void *addr) 
{
    char        dummy;
    int         result;
    int         touched;

    mmuInTouch = TRUE;
    touched = TRUE;
    debug("Touch 0x%p\n", addr);
    result = sigsetjmp(mmuTouchBuf, 1);
    if (result == 0) {
        dummy = * ((char *) addr);
        touched = TRUE;
    } else {
        touched = FALSE;
    }
    debug("touch %s\n", (touched == TRUE) ? "succeeded" : "failed"); 
    mmuInTouch = FALSE;
    return touched;
}

/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuSetPageTable
 *
 *      Sets or updates the current page table.
 *
 * Results:
 *      MMU return status.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

int
USLOSS_MmuSetPageTable(USLOSS_PTE *pageTable) 
{
    int         numPages;
    int         status;
    int         i;

    check_kernel_mode("USLOSS_MmuSetPageTable");
    if (mmuPtr == NULL) {
        return USLOSS_MMU_ERR_OFF;
    }
    if (mmuPtr->mode != USLOSS_MMU_MODE_PAGETABLE) {
        return USLOSS_MMU_ERR_MODE;
    }
    numPages = mmuPtr->numPages;

    mmuPtr->pageTable = pageTable;

    // Remove existing mappings.

    for (i = 0; i < numPages; i++) {
        status = Unmap(0, i);
        switch (status) {
            case USLOSS_MMU_OK:
            case USLOSS_MMU_ERR_NOMAP:
                // Everything ok.
                break;
            default:
                // Something is wrong
                goto done;
        }
    }

    if (pageTable != NULL) {
        // Add new mappings.
        for (i = 0; i < numPages; i++) {
            if (pageTable[i].incore) {
                int protection = USLOSS_MMU_PROT_NONE;
                if ((pageTable[i].read == 1) && (pageTable[i].write == 0)) {
                    protection = USLOSS_MMU_PROT_READ;
                } else if ((pageTable[i].read == 1) && (pageTable[i].write == 1)) {
                    protection = USLOSS_MMU_PROT_RW;
                } else if ((pageTable[i].read == 0) && (pageTable[i].write == 0)) {
                    protection = USLOSS_MMU_PROT_NONE;
                } else {
                    status = USLOSS_MMU_ERR_PROT;
                    goto done;
                }
                status = Map(0, i, pageTable[i].frame, protection);
                if (status == USLOSS_MMU_ERR_FRAME) {
                    USLOSS_Console("USLOSS_MmuSetPageTable: Page %d has invalid frame %u\n", i, 
                                   pageTable[i].frame);
                }
                if (status != USLOSS_MMU_OK) {
                    goto done;
                }
            }
        }
    }
    status = USLOSS_MMU_OK;
done:
    return status;
}

/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuGetPageTable
 *
 *      Gets the current page table.
 *
 * Results:
 *      MMU return status.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

int
USLOSS_MmuGetPageTable(USLOSS_PTE **pageTable) 
{
    check_kernel_mode("USLOSS_MmuGetPageTable");
    if (mmuPtr == NULL) {
        return USLOSS_MMU_ERR_OFF;
    }
    if (mmuPtr->mode != USLOSS_MMU_MODE_PAGETABLE) {
        return USLOSS_MMU_ERR_MODE;
    }
    *pageTable = mmuPtr->pageTable;
    return USLOSS_MMU_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * USLOSS_MmuGetMode
 *
 *      Gets MMU mode.
 *
 * Results:
 *      MMU return status.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

int
USLOSS_MmuGetMode(int *mode) 
{
    check_kernel_mode("USLOSS_MmuGetMode");
    if (mmuPtr == NULL) {
        return USLOSS_MMU_ERR_OFF;
    }
    *mode = mmuPtr->mode;
    return USLOSS_MMU_OK;
}
