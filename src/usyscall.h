
#ifndef _USYSCALL_H
#define _USYSCALL_H

// DO NOT MODIFY THIS FILE.

/*  Different possible syscall opcodes */

#define SYS_TERMREAD		1
#define SYS_TERMWRITE		2
#define SYS_SPAWN		3
#define SYS_WAIT		4
#define SYS_TERMINATE		5
#define SYS_MBOXCREATE		6
#define SYS_MBOXRELEASE		7
#define SYS_MBOXSEND		8
#define SYS_MBOXRECEIVE		9
#define SYS_MBOXCONDSEND	10
#define SYS_MBOXCONDRECEIVE	11
#define SYS_SLEEP		12
#define SYS_DISKREAD		13
#define SYS_DISKWRITE		14
#define SYS_DISKSIZE		15
#define SYS_SEMCREATE		16
#define SYS_SEMP		17
#define SYS_SEMV		18
#define SYS_SEMFREE		19
#define SYS_GETTIMEOFDAY	20
#define SYS_CPUTIME		21
#define SYS_GETPID		22
#define SYS_DUMPPROCESSES       23

#ifdef PHASE_3
#define SYS_VMINIT		24
#define SYS_VMDESTROY		25
#define SYS_HEAPALLOC		26
#define SYS_HEAPFREE		27
#define SYS_PROTECT		28
#define SYS_SHARE		29
#define SYS_COW			30
#endif

// Leave some room for growth

#define USLOSS_MAX_SYSCALLS	40


/*  The USLOSS_Sysargs structure */
typedef struct USLOSS_Sysargs
{
	int number;
	void *arg1;
	void *arg2;
	void *arg3;
	void *arg4;
	void *arg5;
} USLOSS_Sysargs;


#endif	/*  _SYSCALL_H */

