/*
 * This file contains the function definitions for the library interfaces
 * to the USLOSS system calls.
 */
#ifndef _LIBUSER_H
#define _LIBUSER_H

#ifndef CHECKRETURN
#define CHECKRETURN __attribute__((warn_unused_result))
#endif


extern int Sys_TermRead(char *buff, int bsize, int unit, int *nread) CHECKRETURN; 
extern int Sys_TermWrite(char *buff, int bsize, int unit, int *nwrite) CHECKRETURN;    
extern int Sys_Spawn(char *name, int (*func)(void *), void *arg, int stack_size, 
        int priority, int *pid) CHECKRETURN;   
extern int Sys_Wait(int *pid, int *status) CHECKRETURN;
extern int Sys_Terminate(int status);
extern int Sys_Sleep(int seconds) CHECKRETURN;                  
extern int Sys_DiskWrite(void *dbuff, int first,int sectors,int unit)
            CHECKRETURN;
extern int Sys_DiskRead(void *dbuff, int first, int sectors,int unit)
            CHECKRETURN;
extern int Sys_DiskSize(int unit, int *sector, int *disk) CHECKRETURN;
extern int Sys_GetTimeOfDay(int *tod);                           
extern int Sys_GetProcInfo(int pid, void *info) CHECKRETURN;                     
extern int Sys_GetPid(int *pid);         
extern int Sys_SemName(int semaphore, char *name)  CHECKRETURN;              
extern int Sys_SemCreate(char *name, int value, int *semaphore) CHECKRETURN;
extern int Sys_SemP(int semaphore) CHECKRETURN;
extern int Sys_SemV(int semaphore) CHECKRETURN;
extern int Sys_SemFree(int semaphore) CHECKRETURN;
extern int Sys_LockCreate(char *name, int *lid) CHECKRETURN;
extern int Sys_LockFree(int lid) CHECKRETURN;
extern int Sys_LockName(int lid, char *name, int len) CHECKRETURN;
extern int Sys_LockAcquire(int lid) CHECKRETURN;
extern int Sys_LockRelease(int lid) CHECKRETURN;
extern int Sys_CondCreate(char *name, int lid, int *vid) CHECKRETURN;
extern int Sys_CondFree(int vid) CHECKRETURN;
extern int Sys_CondName(int vid, char *name, int len) CHECKRETURN;
extern int Sys_CondWait(int vid) CHECKRETURN;
extern int Sys_CondSignal(int vid) CHECKRETURN;
extern int Sys_CondBroadcast(int vid) CHECKRETURN;

extern int Sys_VmInit(int mappings, int pages, int frames, int pagers, void **region, 
                      int *pageSize) CHECKRETURN;
extern int Sys_VmShutdown(void);
/*
 * Phase 3 extra credit.
 */
extern int Sys_Protect(int page, int protection);
extern int Sys_Share(int pid, int source, int target);
extern int Sys_COW(int pid, int source, int target);

extern int Sys_MboxCreate(int numslots, int slotsize, int *mbox) CHECKRETURN;
extern int Sys_MboxRelease(int mbox) CHECKRETURN;
extern int Sys_MboxSend(int mbox, void *msg, int *size) CHECKRETURN;                     
extern int Sys_MboxReceive(int mbox, void *msg, int *size) CHECKRETURN;
extern int Sys_MboxCondSend(int mbox, void *msg, int *size) CHECKRETURN;                     
extern int Sys_MboxCondReceive(int mbox, void *msg, int *size) CHECKRETURN;

#endif

