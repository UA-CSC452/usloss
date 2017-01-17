/*
 * mmuInt.h
 *
 *	Internal declarations for the USLOSS mmu.
 */
#ifndef _MMU_INT_H
#define _MMU_INT_H
#include <setjmp.h>
#include <ucontext.h>

extern void 	USLOSS_MmuHandler(int sig, siginfo_t *sigstuff, ucontext_t *old_context);
extern int      USLOSS_MmuGetMode(int *mode) __attribute__((warn_unused_result));

extern int	mmuInTouch;
extern jmp_buf	mmuTouchBuf;

#endif

