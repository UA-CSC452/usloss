
#ifndef _USYSCALL_H
#define _USYSCALL_H

// DO NOT MODIFY THIS FILE.

/*  Different possible syscall opcodes */

#define SYS_TERMREAD        1
#define SYS_TERMWRITE       2
#define SYS_SPAWN           3
#define SYS_WAIT            4
#define SYS_TERMINATE       5
#define SYS_MBOXCREATE      6
#define SYS_MBOXRELEASE     7
#define SYS_MBOXSEND        8
#define SYS_MBOXRECEIVE     9
#define SYS_MBOXCONDSEND    10
#define SYS_MBOXCONDRECEIVE 11
#define SYS_SLEEP           12
#define SYS_DISKREAD        13
#define SYS_DISKWRITE       14
#define SYS_DISKSIZE        15
#define SYS_SEMCREATE       16
#define SYS_SEMP            17
#define SYS_SEMV            18
#define SYS_SEMFREE         19
#define SYS_GETTIMEOFDAY    20
#define SYS_GETPROCINFO     21
#define SYS_GETPID          22
#define SYS_SEMNAME         23
#define SYS_LOCKCREATE      24
#define SYS_LOCKFREE        25
#define SYS_LOCKNAME        26
#define SYS_LOCK            27
#define SYS_UNLOCK          28
#define SYS_CONDCREATE      29
#define SYS_CONDFREE        30
#define SYS_CONDNAME        31
#define SYS_CONDWAIT        32
#define SYS_SIGNAL          33
#define SYS_NAKEDSIGNAL     34
#define SYS_BROADCAST       35

#define SYS_VMINIT          36
#define SYS_VMSHUTDOWN      37
#define SYS_HEAPALLOC       38
#define SYS_HEAPFREE        39
#define SYS_PROTECT         40
#define SYS_SHARE           41
#define SYS_COW             42

// Leave some room for growth

#define USLOSS_MAX_SYSCALLS 50


/*  The USLOSS_Sysargs structure */
typedef struct USLOSS_Sysargs
{
    unsigned int number;
    void *arg1;
    void *arg2;
    void *arg3;
    void *arg4;
    void *arg5;
} USLOSS_Sysargs;


#endif  /*  _SYSCALL_H */

