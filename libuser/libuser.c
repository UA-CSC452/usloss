/*
 *  File:  libuser.c
 *
 *  Description:  This file contains the interface declarations
 *                to the OS kernel support package.
 *
 */

#include <libuser.h>
#include <usloss.h>
#include <usyscall.h>

#define CHECKMODE {						\
	if (USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) { 				\
	    USLOSS_Console("Trying to invoke syscall from kernel\n");	\
	    USLOSS_Halt(1);						\
	}							\
}
/*
 *  Routine:  Sys_TermRead
 *
 *  Description: This is the call entry point for terminal input.
 *
 *  Arguments:    char *buff    -- pointer to the input buffer
 *                int   bsize   -- maximum size of the buffer
 *                int   unit_id -- terminal unit number
 *                int  *nread      -- pointer to output value
 *                (output value: number of characters actually read)
 *
 *  Return Value: 0 means success, -1 means error occurs
 *
 */
int Sys_TermRead(char *buff, int bsize, int unit_id, int *nread)     
{
    USLOSS_Sysargs sa;

    CHECKMODE;
    sa.number = SYS_TERMREAD;
    sa.arg1 = (void *) buff;
    sa.arg2 = (void *) bsize;
    sa.arg3 = (void *) unit_id;
    USLOSS_Syscall((void *) &sa);
    *nread = (int) sa.arg2;
    return (int) sa.arg4;
} 

/*
 *  Routine:  Sys_TermWrite
 *
 *  Description: This is the call entry point for terminal output.
 *
 *  Arguments:    char *buff    -- pointer to the output buffer
 *                int   bsize   -- number of characters to write
 *                int   unit_id -- terminal unit number
 *                int  *nwrite      -- pointer to output value
 *                (output value: number of characters actually written)
 *
 *  Return Value: 0 means success, -1 means error occurs
 *
 */
int Sys_TermWrite(char *buff, int bsize, int unit_id, int *nwrite)    
{
    USLOSS_Sysargs sa;
    
    CHECKMODE;
    sa.number = SYS_TERMWRITE;
    sa.arg1 = (void *) buff;
    sa.arg2 = (void *) bsize;
    sa.arg3 = (void *) unit_id;
    USLOSS_Syscall((void *) &sa);
    *nwrite = (int) sa.arg2;
    return (int) sa.arg4;
} 

/*
 *  Routine:  Sys_Spawn
 *
 *  Description: This is the call entry to fork a new user process.
 *
 *  Arguments:    
 *		  char *name    -- process's name
 *		  PFV func      -- pointer to the function to fork
 *		  void *arg	-- argument to function
 *                int stacksize -- amount of stack to be allocated
 *                int priority  -- priority of forked process
 *                int  *pid      -- pointer to output value
 *                (output value: process id of the forked process)
 *
 *  Return Value: 0 means success, -1 means error occurs
 *
 */
int Sys_Spawn(char *name, int (*func)(void *), void *arg, int stack_size, int priority, 
	int *pid)   
{
    USLOSS_Sysargs sa;
    
    CHECKMODE;
    sa.number = SYS_SPAWN;
    sa.arg1 = (void *) func;
    sa.arg2 = arg;
    sa.arg3 = (void *) stack_size;
    sa.arg4 = (void *) priority;
    sa.arg5 = (void *) name;
    USLOSS_Syscall((void *) &sa);
    *pid = (int) sa.arg1;
    return (int) sa.arg4;
} 

/*
 *  Routine:  Sys_Wait
 *
 *  Description: This is the call entry to wait for a child completion
 *
 *  Arguments:    int *pid -- pointer to output value 1
 *                (output value 1: process id of the completing child)
 *                int *status -- pointer to output value 2
 *                (output value 2: status of the completing child)
 *
 *  Return Value: 0 means success, -1 means error occurs
 *
 */
int Sys_Wait(int *pid, int *status)	
{
    USLOSS_Sysargs sa;
    
    CHECKMODE;
    sa.number = SYS_WAIT;
    USLOSS_Syscall((void *) &sa);
    *pid = (int) sa.arg1;
    *status = (int) sa.arg2;
    return (int) sa.arg4;
} 


/*
 *  Routine:  Sys_Terminate
 *
 *  Description: This is the call entry to terminate 
 *               the invoking process and its children
 *
 *  Arguments:   int status -- the completion status of the process
 *
 *  Return Value: 0 means success, -1 means error occurs
 *
 */
void Sys_Terminate(int status)
{
    USLOSS_Sysargs sa;
    
    CHECKMODE;
    sa.number = SYS_TERMINATE;
    sa.arg1 = (void *) status;
    USLOSS_Syscall((void *) &sa);
    return;
    
} 

/*
 *  Routine:  Sys_Sleep
 *
 *  Description: This is the call entry point for timed delay.
 *
 *  Arguments:    int seconds -- number of seconds to sleep
 *
 *  Return Value: 0 means success, -1 means error occurs
 *
 */
int Sys_Sleep(int seconds)                                 
{
    USLOSS_Sysargs sa;
    
    CHECKMODE;
    sa.number = SYS_SLEEP;
    sa.arg1 = (void *) seconds;
    USLOSS_Syscall((void *) &sa);
    return (int) sa.arg4;
} /* end of Delay */

/*
 *  Routine:  Sys_DiskWrite
 *
 *  Description: This is the call entry point for disk output.
 *
 *  Arguments:    void* dbuff  -- pointer to the output buffer
 *                int   track  -- first track to write
 *                int   first -- first sector to write
 *		  int	sectors -- number of sectors to write
 *		  int   unit   -- unit number of the disk
 *                int   *status    -- pointer to output value
 *                (output value: completion status)
 *
 *  Return Value: 0 means success, -1 means error occurs
 *
 */
int Sys_DiskWrite(void *dbuff, int track, int first, int sectors, int unit, int *status)
{
    USLOSS_Sysargs sa;

    CHECKMODE;
    sa.number = SYS_DISKWRITE;
    sa.arg1 = dbuff;
    sa.arg2 = (void *) sectors;
    sa.arg3 = (void *) track;
    sa.arg4 = (void *) first;
    sa.arg5 = (void *) unit;
    USLOSS_Syscall((void *) &sa);
    *status = (int) sa.arg1;
    return (int) sa.arg4;
} /* end of DiskWrite */

/*
 *  Routine:  Sys_DiskRead
 *
 *  Description: This is the call entry point for disk input.
 *
 *  Arguments:    void* dbuff  -- pointer to the input buffer
 *                int   track  -- first track to read 
 *                int   first -- first sector to read
 *		  int	sectors -- number of sectors to read
 *		  int   unit   -- unit number of the disk
 *                int   *status    -- pointer to output value
 *                (output value: completion status)
 *
 *  Return Value: 0 means success, -1 means error occurs
 *
 */
int Sys_DiskRead(void *dbuff, int track, int first, int sectors, int unit, int *status)
{
    USLOSS_Sysargs sa;
    
    CHECKMODE;
    sa.number = SYS_DISKREAD;
    sa.arg1 = dbuff;
    sa.arg2 = (void *) sectors;
    sa.arg3 = (void *) track;
    sa.arg4 = (void *) first;
    sa.arg5 = (void *) unit;
    USLOSS_Syscall((void *) &sa);
    *status = (int) sa.arg1;
    return (int) sa.arg4;
} /* end of DiskRead */

/*
 *  Routine:  Sys_DiskSize
 *
 *  Description: Return information about the size of the disk.
 *
 *  Arguments:    int   unit  -- the unit number of the disk 
 *                int   *sector -- bytes in a sector
 *		  int	*track -- number of sectors in a track
 *                int   *disk  -- number of tracks in the disk
 *                (output value: completion status)
 *
 *  Return Value: 0 means success, -1 means invalid parameter
 *
 */
int Sys_DiskSize(int unit, int *sector, int *track, int *disk)
{
	USLOSS_Sysargs sa;

	CHECKMODE;
	sa.number = SYS_DISKSIZE;
	sa.arg1 = (void *) unit;
	USLOSS_Syscall((void *) &sa);
	*sector = (int) sa.arg1;
	*track = (int) sa.arg2;
	*disk = (int) sa.arg3;
	return (int) sa.arg4;
} /* end of DiskSize */

/*
 *  Routine:  Sys_GetTimeOfDay
 *
 *  Description: This is the call entry point for getting the time of day.
 *
 *  Arguments:    int *tod  -- pointer to output value
 *                (output value: the time of day)
 *
 */
void Sys_GetTimeOfDay(int *tod)                           
{
    USLOSS_Sysargs sa;
    
    CHECKMODE;
    sa.number = SYS_GETTIMEOFDAY;
    USLOSS_Syscall((void *) &sa);
    *tod = (int) sa.arg1;
    return;
} /* end of GetTimeOfDay */

/*
 *  Routine:  Sys_CPUTime
 *
 *  Description: This is the call entry point for the process' CPU time.
 *		
 *
 *  Arguments:    int *cpu  -- pointer to output value
 *                (output value: the CPU time of the process)
 *
 */
void Sys_CPUTime(int *cpu)                           
{
    USLOSS_Sysargs sa;

    CHECKMODE;
    sa.number = SYS_CPUTIME;
    USLOSS_Syscall((void *) &sa);
    *cpu = (int) sa.arg1;
    return;
} /* end of CPUTime */

/*
 *  Routine:  Sys_GetPID
 *
 *  Description: This is the call entry point for the process' PID.
 *		
 *
 *  Arguments:    int *pid  -- pointer to output value
 *                (output value: the PID)
 *
 */
void Sys_GetPID(int *pid)                           
{
    USLOSS_Sysargs sa;

    CHECKMODE;
    sa.number = SYS_GETPID;
    USLOSS_Syscall((void *) &sa);
    *pid = (int) sa.arg1;
    return;
} /* end of GetPID */


/*
 *  Routine:  Sys_DumpProcesses
 *
 *  Description: This is the call entry point for printing the processes.
 *      
 *
 *  Arguments:    None
 *
 */
void Sys_DumpProcesses(void)                           
{
    USLOSS_Sysargs sa;

    CHECKMODE;
    sa.number = SYS_DUMPPROCESSES;
    USLOSS_Syscall((void *) &sa);
    return;
} /* end of DumpProcesses */

/*
 *  Routine:  Sys_SemCreate
 *
 *  Description: Create a semaphore.
 *		
 *
 *  Arguments:    char *name -- semaphore name
 *                int value -- initial semaphore value
 *		          int *semaphore -- semaphore handle
 *                (output value: completion status)
 *
 */
int Sys_SemCreate(char *name, int value, int *semaphore)
{
    USLOSS_Sysargs sa;

    CHECKMODE;
    sa.number = SYS_SEMCREATE;
    sa.arg1 = (void *) value;
    sa.arg2 = (void *) name;
    USLOSS_Syscall((void *) &sa);
    *semaphore = (int) sa.arg1;
    return (int) sa.arg4;
} /* end of SemCreate */


/*
 *  Routine:  Sys_SemP
 *
 *  Description: "P" a semaphore.
 *		
 *
 *  Arguments:    int semaphore -- semaphore handle
 *                (output value: completion status)
 *
 */
int Sys_SemP(int semaphore)
{
    USLOSS_Sysargs sa;

    CHECKMODE;
    sa.number = SYS_SEMP;
    sa.arg1 = (void *) semaphore;
    USLOSS_Syscall((void *) &sa);
    return (int) sa.arg4;
} /* end of SemP */

/*
 *  Routine:  Sys_SemV
 *
 *  Description: "V" a semaphore.
 *		
 *
 *  Arguments:    int semaphore -- semaphore handle
 *                (output value: completion status)
 *
 */
int Sys_SemV(int semaphore)
{
    USLOSS_Sysargs sa;

    CHECKMODE;
    sa.number = SYS_SEMV;
    sa.arg1 = (void *) semaphore;
    USLOSS_Syscall((void *) &sa);
    return (int) sa.arg4;
} /* end of SemV */

/*
 *  Routine:  Sys_SemFree
 *
 *  Description: Free a semaphore.
 *		
 *
 *  Arguments:    int semaphore -- semaphore handle
 *                (output value: completion status)
 *
 */
int Sys_SemFree(int semaphore)
{
    USLOSS_Sysargs sa;

    CHECKMODE;
    sa.number = SYS_SEMFREE;
    sa.arg1 = (void *) semaphore;
    USLOSS_Syscall((void *) &sa);
    return (int) sa.arg4;
} /* end of SemFree */

#ifdef PHASE_3

/*
 *  Routine:  Sys_VmInit
 *
 *  Description: Initializes the VM system.
 *		
 *
 *  Arguments:   int mappings -- number of mappings the MMU can hold
 *               int pages -- number of pages in VM region
 *		 int frames -- number of frames of physical memory
 *	         int pagers -- number of pager daemons to create
 *		 
 *  Return Value: -1 for illegal values, -2 if the region has already been initialized, 
 *                0 otherwise
 */

int Sys_VmInit(int mappings, int pages, int frames, int pagers, void **region)
{
    USLOSS_Sysargs sa;
    int		   rc;

    CHECKMODE;
    sa.number = SYS_VMINIT;
    sa.arg1 = (void *) mappings;
    sa.arg2 = (void *) pages;
    sa.arg3 = (void *) frames;
    sa.arg4 = (void *) pagers;
    USLOSS_Syscall((void *) &sa);
    rc = (int) sa.arg4;
    if (rc == 0) {
	*region = sa.arg1;
    }
    return rc;
}

/*
 *  Routine:  Sys_VmDestroy
 *
 *  Description: Stops the VM system.
 *		
 *
 *  Arguments:   None
 *		 
 *  Return Value: None
 */
void Sys_VmDestroy(void)
{
    USLOSS_Sysargs sa;

    CHECKMODE;
    sa.number = SYS_VMDESTROY;
    USLOSS_Syscall((void *) &sa);
    return;
}



/*
 *  Routine:  Sys_Protect
 *
 *  Description: Protect a page in the VM region (used for Phase 3
 *		 extra credit).
 *		
 *
 *  Arguments:   int page -- page to protect 
 *               int protection -- protection flags (see mmu.h)
 *		 
 *  Return Value: -1 if an error occurred, 0 otherwise
 */

int Sys_Protect(int page, int protection)
{
    USLOSS_Sysargs sa;

    CHECKMODE;
    sa.number = SYS_PROTECT;
    sa.arg1 = (void *) page;
    sa.arg2 = (void *) protection;
    USLOSS_Syscall((void *) &sa);
    return (int) sa.arg4;
} /* end of Protect */

/*
 *  Routine:  Sys_Share
 *
 *  Description: Share a page in the VM region (used for Phase 3
 *		 extra credit). Page "source" in process "pid"
 *		 is shared as page "target" in the calling process.
 *		 The current contents of page "target" are lost.
 *		
 *
 *  Arguments:   int pid -- process with which to share
 *		 int source -- page in process "pid" to share
 *		 int target -- page in current process to share
 *
 *  Return Value: -1 if an error occurred, 0 otherwise
 */

int Sys_Share(int pid, int source, int target)
{
    USLOSS_Sysargs sa;

    CHECKMODE;
    sa.number = SYS_SHARE;
    sa.arg1 = (void *) pid;
    sa.arg2 = (void *) source;
    sa.arg3 = (void *) target;
    USLOSS_Syscall((void *) &sa);
    return (int) sa.arg4;
} /* end of Share */

/*
 *  Routine:  Sys_COW
 *
 *  Description: Copy-on-write a page in the VM region (used for Phase 3
 *		 extra credit). Page "source" in process "pid"
 *		 is shared copy-on-write as page "target" in 
 *		 the calling process.
 *		 The current contents of page "target" are lost.
 *		
 *
 *  Arguments:   int pid -- process with which to share COW
 *		 int source -- page in process "pid" to share COW
 *		 int target -- page in current process to share COW
 *
 *  Return Value: -1 if an error occurred, 0 otherwise
 */

int Sys_COW(int pid, int source, int target)
{
    USLOSS_Sysargs sa;

    CHECKMODE;
    sa.number = SYS_COW;
    sa.arg1 = (void *) pid;
    sa.arg2 = (void *) source;
    sa.arg3 = (void *) target;
    USLOSS_Syscall((void *) &sa);
    return (int) sa.arg4;
} /* end of COW */

/*
 *  Routine:  Sys_HeapAlloc
 *
 *  Description: This is the call entry point allocate from a heap.
 *		
 *
 *  Arguments:    int bytes -- number of bytes to allocate
 *		  void **ptr  -- pointer to allocated memory
 *                (output value: completion status)
 *
 */
int Sys_HeapAlloc(int bytes, void **ptr)                           
{
    USLOSS_Sysargs sa;

    CHECKMODE;
    sa.number = SYS_HEAPALLOC;
    sa.arg1 = (void *) bytes;
    USLOSS_Syscall((void *) &sa);
    *ptr = sa.arg1;
    return (int) sa.arg4;
} /* end of HeapAlloc */

/*
 *  Routine:  Sys_HeapFree
 *
 *  Description: This is the call entry point free a block of memory.
 *		
 *
 *  Arguments:    void *ptr -- block to free
 *
 */
int Sys_HeapFree(void *ptr)
{
    USLOSS_Sysargs sa;

    CHECKMODE;
    sa.number = SYS_HEAPFREE;
    sa.arg1 = ptr;
    USLOSS_Syscall((void *) &sa);
    return (int) sa.arg4;
} /* end of HeapFree */

#endif 

/*
 *  Routine:  Sys_MboxCreate
 *
 *  Description: This is the call entry point to create a new mail box.
 *
 *  Arguments:    int   numslots -- number of mailbox slots
 *                int   slotsize -- size of the mailbox buffer
 *                int  *mid      -- pointer to output value
 *                (output value: id of created mailbox)
 *
 *  Return Value: 0 means success, -1 means error occurs
 *
 */
int Sys_MboxCreate(int numslots, int slotsize, int *mid)
{
    USLOSS_Sysargs sa;
    
    CHECKMODE;
    sa.number = SYS_MBOXCREATE;
    sa.arg1 = (void *) numslots;
    sa.arg2 = (void *) slotsize;
    USLOSS_Syscall((void *) &sa);
    *mid = (int) sa.arg1;
    return (int) sa.arg4;
} /* end of MboxCreate */

/*
 *  Routine:  Sys_MboxRelease
 *
 *  Description: This is the call entry point to release a mailbox
 *
 *  Arguments: int mbox  -- id of the mailbox
 *
 *  Return Value: 0 means success, -1 means error occurs
 *
 */
int Sys_MboxRelease(int mbox)
{
    USLOSS_Sysargs sa;
    
    CHECKMODE;
    sa.number = SYS_MBOXRELEASE;
    sa.arg1 = (void *) mbox;
    USLOSS_Syscall((void *) &sa);
    return (int) sa.arg4;
} /* end of MboxRelease */


/*
 *  Routine:  Sys_MboxSend
 *
 *  Description: This is the call entry point mailbox send.
 *
 *  Arguments:    int mbox -- id of the mailbox to send to
 *                void* msg  -- message to send
 *		  int *size -- size of message 
 *
 *  Return Value: 0 means success, -1 means error occurs
 *
 */
int Sys_MboxSend(int mbox, void *msg, int *size)                     
{
    USLOSS_Sysargs 	sa;

    CHECKMODE;
    sa.number = SYS_MBOXSEND;
    sa.arg1 = (void *) mbox;
    sa.arg2 = msg;
    sa.arg3 = (void *) *size;
    USLOSS_Syscall((void *) &sa);
    *size = (int) sa.arg3;
    return (int) sa.arg4;
} /* end of MboxSend */

/*
 *  Routine:  Sys_MboxReceive
 *
 *  Description: This is the call entry point for terminal input.
 *
 *  Arguments:    int mbox -- id of the mailbox to receive from
 *                void* msg  -- location to receive message
 *		  int *size -- size of message 
 *
 *  Return Value: -1 if an error occurred, otherwise the size of the received message 
 *
 */
int Sys_MboxReceive(int mbox, void *msg, int *size)
{
    USLOSS_Sysargs 	sa;

    CHECKMODE;
    sa.number = SYS_MBOXRECEIVE;
    sa.arg1 = (void *) mbox;
    sa.arg2 = msg;
    sa.arg3 = (void *) *size;
    USLOSS_Syscall( (void *) &sa );
    *size = (int) sa.arg2;
    return (int) sa.arg4;
} /* end of MboxReceive */

/*
 *  Routine:  Sys_MboxCondSend
 *
 *  Description: This is the call entry point mailbox conditional send.
 *
 *  Arguments:    int mbox -- id of the mailbox to send to
 *                void* msg  -- message to send
 *		  int *size -- size of message 
 *
 *  Return Value: 0 means success, -1 means error occurs, 1 means mailbox is full
 *
 */
int Sys_MboxCondSend(int mbox, void *msg, int *size)                     
{
    USLOSS_Sysargs sa;

    CHECKMODE;
    sa.number = SYS_MBOXCONDSEND;
    sa.arg1 = (void *) mbox;
    sa.arg2 = msg;
    sa.arg3 = (void *) *size;
    USLOSS_Syscall((void *) &sa);
    *size = (int) sa.arg3;
    return (int) sa.arg4;
}

/*
 *  Routine:  Sys_MboxCondReceive
 *
 *  Description: This is the call entry point for terminal input.
 *
 *  Arguments:    int mbox -- id of the mailbox to receive from
 *                void* msg  -- message to send
 *		  int *size -- size of message 
 *
 *  Return Value: -1 if an error occurred, otherwise the size of the received message 
 *
 */
int Sys_MboxCondReceive(int mbox, void *msg, int *size)
{
    USLOSS_Sysargs 	sa;

    CHECKMODE;
    sa.number = SYS_MBOXCONDRECEIVE;
    sa.arg1 = (void *) mbox;
    sa.arg2 = msg;
    sa.arg3 = (void *) *size;
    USLOSS_Syscall( (void *) &sa );
    *size = (int) sa.arg2;
    return (int) sa.arg4;
} 


/* end libuser.c */
