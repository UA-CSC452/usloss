/*
 * This file contains the function definitions for the library interfaces
 * to the USLOSS system calls.
 */
#ifndef _LIBUSER_H
#define _LIBUSER_H

extern int Sys_TermRead(char *buff, int bsize, int unit, int *nread); 
extern int Sys_TermWrite(char *buff, int bsize, int unit, int *nwrite);    
extern int Sys_Spawn(char *name, int (*func)(void *), void *arg, int stack_size, 
		int priority, int *pid);   
extern int Sys_Wait(int *pid, int *status);
extern void Sys_Terminate(int status);
extern int Sys_Sleep(int seconds);                  
extern int Sys_DiskWrite(void *dbuff, int track, int first,int sectors,int unit, int *status);
extern int Sys_DiskRead(void *dbuff, int track, int first, int sectors,int unit, int *status);
extern int Sys_DiskSize(int unit, int *sector, int *track, int *disk);
extern void Sys_GetTimeOfDay(int *tod);                           
extern void Sys_CPUTime(int *cpu);                      
extern void Sys_GetPID(int *pid);         
extern void Sys_DumpProcesses(void);                
extern int Sys_SemCreate(int value, int *semaphore);
extern int Sys_SemP(int semaphore);
extern int Sys_SemV(int semaphore);
extern int Sys_SemFree(int semaphore);

#ifdef PHASE_3
extern int Sys_VmInit(int mappings, int pages, int frames, int pagers, void **region);
extern void Sys_VmDestroy(void);
/*
 * Phase 3 extra credit.
 */
extern int Protect(int page, int protection);
extern int Share(int pid, int source, int target);
extern int COW(int pid, int source, int target);
#endif

extern int Sys_MboxCreate(int numslots, int slotsize, int *mbox);
extern int Sys_MboxRelease(int mbox);
extern int Sys_MboxSend(int mbox, void *msg, int *size);                     
extern int Sys_MboxReceive(int mbox, void *msg, int *size);
extern int Sys_MboxCondSend(int mbox, void *msg, int *size);                     
extern int Sys_MboxCondReceive(int mbox, void *msg, int *size);

#endif

