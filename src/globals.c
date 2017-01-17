
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include "project.h"
#include "globals.h"
#include "main.h"
#include "sig_ints.h"
#include "usloss.h"

dynamic_def(unsigned int current_psr = USLOSS_PSR_MAGIC);
dynamic_def(int pclock_ticks);
dynamic_def(int partial_ticks);
dynamic_def(volatile int USLOSSwaiting);
char *usloss_version = VERSION;

dynamic_fun void globals_init(void)
{
    USLOSSwaiting = 0;
    current_psr |= USLOSS_PSR_CURRENT_MODE;/* Start in kernel mode, interrupts off */
    pclock_ticks = 0;
    partial_ticks = 0;
}
void check_interrupts(void) {

#ifdef NOTDEF
    sigset_t cur_set;
    int err_return;
    int on;

    /*
     * DOES NOTHING FOR THE MOMENT.
     */
    debug("check_interrupts: psr = 0x%x\n", current_psr);
    /*
     * This code is to verify that the interrupt value in the psr 
     * corresponds to the signal mask. If there is a mismatch then
     * I didn't implement the psr properly. JHH 1/28/97.
     */
    if ((current_psr & ~USLOSS_PSR_MASK) != USLOSS_PSR_MAGIC) {
	usloss_assert(0, "corrupted psr");
    }
    on = sigismember(&sim_set, SIG_ALARM) ? 0 : 1;
    if (((current_psr & USLOSS_PSR_CURRENT_INT) >> 1) != on) {
	usloss_assert(0, "psr interrupt wrong");
    }
#endif
    return;
}
void psr_valid(void) 
{
    if ((current_psr & ~USLOSS_PSR_MASK) != USLOSS_PSR_MAGIC) {
	usloss_assert(0, "corrupted psr");
    }
}

unsigned int USLOSS_PsrGet(void) 
{
    unsigned int result;
    int enabled;

    enabled = int_off();
    check_interrupts();
    psr_valid();
    result = current_psr & USLOSS_PSR_MASK;
    if (enabled) {
	int_on();
    }
    return result;
}


int USLOSS_PsrSet(unsigned int new)
{
    int status;
    check_kernel_mode("USLOSS_PsrSet");
    (void) int_off();
    check_interrupts();
    psr_valid();
    if (new & ~USLOSS_PSR_MASK) {
        status = USLOSS_ERR_INVALID_PSR;
        goto done;
    }
    if ((new & USLOSS_PSR_CURRENT_MASK) == 0) {
        status = USLOSS_ERR_INVALID_PSR;
        goto done;
    }
    current_psr = USLOSS_PSR_MAGIC | new;
    if (current_psr & USLOSS_PSR_CURRENT_INT) {
	   int_on();
    }
    check_interrupts();
    status = USLOSS_ERR_OK;
done:
    return status;
}

/*
 *  Outputs a printf-style formatted string to stderr
 */
void USLOSS_Trace(char *fmt, ...)
{
    va_list ap;
    int enabled;

    enabled = int_off();
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fflush(stderr);
    va_end(ap);
    if (enabled) {
	int_on();
    }
}
/*
 *  Outputs a printf-style formatted string to stdout
 */
void USLOSS_Console(char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    USLOSS_VConsole(fmt, ap);
    va_end(ap);
}
void USLOSS_VConsole(char *fmt, va_list ap)
{
    int enabled;

    enabled = int_off();
    vfprintf(stdout, fmt, ap);
    fflush(stdout);
    if (enabled) {
	   int_on();
    }
}

/*
 *  Returns the system clock time (# of microseconds since the kernel started)
 */
int USLOSSClock(void)
{
    int value;
    int enabled;

    check_kernel_mode("USLOSS_Clock");
    enabled = int_off();
    partial_ticks += atleast(5);
    if (partial_ticks >= ALARM_TIME) {
	   pclock_ticks++;
	   partial_ticks -= ALARM_TIME;
    }
    value =  pclock_ticks * ALARM_TIME + partial_ticks;  /* syscalls per tick */
    if (enabled) {
	   int_on();
    }
    return value;
}

/*
 *  Stops the simulator - called by the operating system
 */
void USLOSS_Halt(int status)
{
    int err_return;

    check_kernel_mode("USLOSS_Halt");
    (void) int_off();
    finish_status = status;
    err_return = setcontext(&finish_context.context);	
    /*  Should never pass here */
    usloss_sys_assert(err_return != -1, "error resuming finishing context");
    exit(0);
}

/*
 *  Aborts the simulator - called by the operating system
 */
void USLOSS_Abort(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    check_kernel_mode("USLOSS_Abort");
    (void) int_off();
    USLOSS_VConsole(fmt, ap);

    abort();
}

/*
 *  This routine, used by the usloss_sys_assert() macro, is called when an
 *  error is detected in a system call or C library routine.  It prints an
 *  error message and forces a core dump.
 */
dynamic_fun void rpt_err(char *file, int line, char *msg)
{
    fprintf(stderr, "INTERNAL USLOSS %s ERROR (%s:%d): ", 
	usloss_version, file, line);
    perror(msg);
    abort();
}

/*
 *  This routine is called when a USLOSS internal inconsistency error is
 *  detected.  It prints an error message and forces a core dump.
 */
dynamic_fun void vrpt_cond(char *msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    fprintf(stderr, "INTERNAL USLOSS %s ERROR: ", usloss_version);
    vfprintf(stderr, msg, ap);
    fprintf(stdout, "\n");
    fflush(stdout);
    va_end(ap);
    abort();
}

/*
 *  This routine, used by the usloss_assert() macro, should be called when
 *  a USLOSS internal inconsistency error is detected.  It prints an error
 *  message and forces a core dump.
 */
dynamic_fun void rpt_cond(char *cond, char *file, int line, char *msg)
{
    fprintf(stderr, "INTERNAL USLOSS %s ERROR(%s,%d): %s !(%s)\n",
	    usloss_version, file, line, msg, cond);
    abort();
}

/*
 *  This routine, used by the usloss_usr_assert() macro (often via the
 *  check_kernel_mode() macro) is called when a user program forces a
 *  simulator trap (such as calling a kernel-mode only routine while not
 *  in kernel mode).  It prints an error message and forces a core dump.
 */
dynamic_fun void rpt_sim_trap(char *msg)
{
    fprintf(stderr, "SIMULATOR TRAP: %s\n", msg);
    abort();
}

/*
 *  Returns a random number between n and 2*n-1 inclusive.  Used to provide
 *  variation in the time required by devices to perform their services.
 */
dynamic_fun int atleast(int n)
{
    return n + (rand() % n);	/*  Can make this better */
}

