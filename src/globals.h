
#if !defined(_globals_h)
#define _globals_h

#include "project.h"
#include <signal.h>

dynamic_dcl volatile int USLOSSwaiting;
dynamic_dcl unsigned int current_psr;
dynamic_dcl int pclock_ticks;
dynamic_dcl int partial_ticks;
dynamic_dcl int partial_ticks;
dynamic_dcl struct sigaction	old_actions[];
dynamic_dcl int dumpcore;

#define USLOSS_PSR_MAGIC 0x45200

#ifdef VIRTUAL_TIME
#define SIG_ALARM SIGVTALRM
#else
#define SIG_ALARM SIGALRM
#endif

#define TRUE 1
#define FALSE 0

dynamic_dcl void globals_init(void);
dynamic_dcl void rpt_err(char *file, int line, char *msg);
dynamic_dcl void rpt_cond(char *cond, char *file, int line, char *msg);
dynamic_dcl void vrpt_cond(char *msg, ...);
dynamic_dcl void rpt_sim_trap(char *msg);
dynamic_dcl int atleast(int num);
dynamic_dcl void check_interrupts(void);
dynamic_dcl void debug(char *msg, ...);
dynamic_dcl void psr_valid(void);
dynamic_dcl int USLOSSClock(void);

#define usloss_sys_assert(EX, STR) \
        (void)((EX) || (rpt_err(__FILE__, __LINE__, STR), 0))
#define usloss_assert(COND, STR) \
        (void)((COND) || (rpt_cond(#COND, __FILE__, __LINE__, STR), 0))
#define usloss_usr_assert(EX, STR) \
        (void)((EX) || (rpt_sim_trap(STR), 0))
#define check_kernel_mode(a) \
        if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0) { \
            USLOSS_IllegalInstruction(); \
        } 

#endif	/*  _globals_h */

