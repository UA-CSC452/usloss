/*
 * These are the definitions for Phase 1 of the project (the kernel).
 * Version 1.8
 * DO NOT MODIFY THIS FILE.
 */

#ifndef _PHASE1_H
#define _PHASE1_H

#include "usloss.h"
#include "usyscall.h"

/*
 * Maximum number of processes.
 */

#define P1_MAXPROC  50

/*
 * Maximum number of semaphores.
 */

#define P1_MAXSEM   2000

/*
 * Maximum number of tags.
 */
#define P1_MAXTAG 2

/*
 * Maximum length of a process or semaphore name, not including
 * trailing '\0'.
 */
#define P1_MAXNAME 80
 

/*
 * Booleans
 */
#define FALSE 0
#define TRUE 1
 
#ifndef CHECKRETURN
#define CHECKRETURN __attribute__((warn_unused_result))
#endif

/*
 * Different states of a process/PCB.
 */
typedef enum P1_State {
    P1_STATE_FREE = 0,      // PCB is not in use
    P1_STATE_RUNNING,       // process is currently running
    P1_STATE_READY,         // process is ready (runnable)
    P1_STATE_QUIT,          // process has quit
    P1_STATE_BLOCKED,       // process is blocked on a semaphore
    P1_STATE_JOINING        // process is waiting for a child to quit
} P1_State;

/*
 * Structure returned by P1_GetProcInfo.
 */
typedef struct P1_ProcInfo {
    char        name[P1_MAXNAME+1];
    P1_State    state;                  // process's state
    int         sid;                    // semaphore on which process is blocked, if any
    int         priority;               // process's priority
    int         tag;                    // process's tag
    int         cpu;                    // CPU consumed (in us)
    int         parent;                 // parent PID
    int         children[P1_MAXPROC];   // childen PIDs
    int         numChildren;            // # of children
} P1_ProcInfo;



/*
 * External function prototypes for this phase.
 */

// Phase1b
extern  int             P1_Fork(char *name, int(*func)(void *), void *arg,
                                int stackSize, int priority, int tag, int *pid) CHECKRETURN;
extern  void            P1_Quit(int status);
extern  int             P1_GetPid(void) CHECKRETURN;
extern  int             P1_GetProcInfo(int pid, P1_ProcInfo *info) CHECKRETURN;

extern  int             P1_Join(int tag, int *pid, int *status) CHECKRETURN;

extern int              P1_SemCreate(char *name, unsigned int value, int *sid) CHECKRETURN;

extern  int             P1_SemFree(int sid) CHECKRETURN;
extern  int             P1_P(int sid) CHECKRETURN;
extern  int             P1_V(int sid) CHECKRETURN;
extern  int             P1_SemName(int sid, char *name) CHECKRETURN; 

extern  int             P1_WaitDevice(int type, int unit, int *status) CHECKRETURN;
extern int              P1_WakeupDevice(int type, int unit, int status, int abort) CHECKRETURN;

extern  int             P2_Startup(void *arg) CHECKRETURN;

/*
 * Error codes
 */

#define P1_SUCCESS 0
#define P1_TOO_MANY_PROCESSES -1
#define P1_TOO_MANY_CONTEXTS P1_TOO_MANY_PROCESSES
#define P1_INVALID_STACK -2
#define P1_INVALID_PRIORITY -3
#define P1_INVALID_TAG -4
#define P1_NO_CHILDREN -5
#define P1_NO_QUIT -6
#define P1_TOO_MANY_SEMS -7
#define P1_NAME_IS_NULL -8
#define P1_DUPLICATE_NAME -9
#define P1_INVALID_SID -10
#define P1_BLOCKED_PROCESSES -11
#define P1_INVALID_PID -12
#define P1_INVALID_CID P1_INVALID_PID
#define P1_INVALID_STATE -13
#define P1_INVALID_TYPE -14
#define P1_INVALID_UNIT -15
#define P1_WAIT_ABORTED -16
#define P1_CHILD_QUIT -17
#define P1_NAME_TOO_LONG -18
#define P1_CONTEXT_IN_USE -19

#endif /* _PHASE1_H */
