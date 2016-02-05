
#ifndef _SYSCALL_H
#define _SYSCALL_H

// DO NOT MODIFY THIS FILE.

/*  Different possible syscall opcodes */

#define SYS_TERMREAD		1
#define SYS_TERMWRITE		2
#define SYS_SPAWN		3
#define SYS_WAIT		4
#define SYS_TERMINATE		5
#define SYS_SLEEP		6
#define SYS_DISKREAD		7
#define SYS_DISKWRITE		8
#define SYS_GETTIMEOFDAY	9
#define SYS_CPUTIME		10
#define SYS_GETPID		11
#define SYS_HEAPALLOC		12
#define SYS_HEAPFREE		13
#define SYS_SEMCREATE		14
#define SYS_SEMP		15
#define SYS_SEMV		16
#define SYS_SEMFREE		17
#define SYS_PROTECT		18
#define SYS_SHARE		19
#define SYS_COW			20

/*
 * Leave some room for growth.
 */

#ifdef MAILBOX
#define SYS_MBOXCREATE		25
#define SYS_MBOXRELEASE		26
#define SYS_MBOXSEND		27
#define SYS_MBOXRECEIVE		28
#define SYS_MBOXCONDSEND	29
#define SYS_MBOXCONDRECEIVE	30

#endif

#define NUM_SYSCALLS		30	


/*  The sysargs structure */
typedef struct sysargs
{
	int number;
	void *arg1;
	void *arg2;
	void *arg3;
	void *arg4;
} sysargs;

extern void USLOSS_Syscall(sysargs *sa);

#endif	/*  _SYSCALL_H */

