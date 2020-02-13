
#if !defined(_globals_h)
#define _globals_h

#include "project.h"
#include "usloss.h"
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>

dynamic_dcl volatile int USLOSSwaiting;
dynamic_dcl unsigned int current_psr;
dynamic_dcl int pclock_ticks;
dynamic_dcl int partial_ticks;
dynamic_dcl int partial_ticks;
dynamic_dcl struct sigaction	old_actions[];
dynamic_dcl int dumpcore;

#define USLOSS_PSR_MAGIC 0x45200

extern int virtual_time;
extern int SIG_ALARM;

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

// Global Verbosity Level and Logging
extern int verbosity;
static inline void LOG(int level, char *fmt, ...)
{
    if (verbosity >= (level)) {
        struct timeval now;
        char date[100];
        char ms[10];
        gettimeofday(&now, 0);
        strftime(date, sizeof(date), "%b %d %H:%M:%S.", localtime(&now.tv_sec));
        snprintf(ms, sizeof(ms), "%03d", (int) (now.tv_usec / 1000));
        strncat(date, ms, sizeof(date) - sizeof(ms) - 1);
        USLOSS_Trace("[%s] USLOSS: ", date);

        va_list ap;
        va_start(ap, fmt);
        USLOSS_VTrace(fmt, ap);
        va_end(ap);
    }
}

// Verbosity Levels
#define PSR_SET_VERBOSITY 4
#define CLOCK_VERBOSITY 3
#define INT_VERBOSITY 3
#define CTX_SWITCH_VERBOSITY 2
#define CTX_INIT_VERBOSITY 1


#endif	/*  _globals_h */

