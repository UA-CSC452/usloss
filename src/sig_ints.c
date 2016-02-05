
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include "project.h"
#include "globals.h"
#include "usloss.h"
#include "sig_ints.h"
#include "devices.h"
#ifdef MMU
#include "mmuInt.h"
#endif
#include <fcntl.h>

#include <sys/time.h>

#define NUM_SIG 100

static void             *syscall_arg = NULL;
static int              syscall_pending = 0;
struct sigaction        old_actions[NUM_SIG];

static USLOSS_Context           *launch_context;

/*  
 *  Timer setup code.
 */

#define ALARM_TIME 10000

dynamic_fun void set_timer(void)
{
    static struct itimerval value, ovalue;

    /*  Set up virtual interrupt timer */
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = ALARM_TIME;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = ALARM_TIME;
#ifdef VIRTUAL_TIME
    setitimer(ITIMER_VIRTUAL, &value, &ovalue);
#else
    setitimer(ITIMER_REAL, &value, &ovalue);
#endif

}

dynamic_fun void stop_timer(void)
{
    static struct itimerval value, ovalue;

    /*  Loading it_value with zeroes stops the timer */
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 0;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 0;
#ifdef VIRTUAL_TIME
    setitimer(ITIMER_VIRTUAL, &value, &ovalue);
#else
    setitimer(ITIMER_REAL, &value, &ovalue);
#endif
}

static void launcher(void) {
    unsigned int psr;
    void (*func)(void);

    assert(launch_context != NULL);
    psr = launch_context->initial_psr;
    func = launch_context->start;
    current_psr = USLOSS_PSR_MAGIC | psr;
    if (psr & USLOSS_PSR_CURRENT_INT) {
       (void) int_on();
    }
    launch_context = NULL;
    (*func)();
    rpt_sim_trap("forked function returned!\n");
}


/*
 *  Routine called by client programs in kernel mode to set up the starting
 *  state of a thread
 */
void USLOSS_ContextInit(USLOSS_Context *ctx, unsigned int psr, char *stack, int stackSize,
    void (*pc)(void))
{
    int err_return;
    int enabled;

    enabled = int_off();
    check_kernel_mode("USLOSS_ContextInit");
    if (psr & ~USLOSS_PSR_MASK) {
        rpt_sim_trap("USLOSS_ContextInit: invalid psr value\n");
    }
    if (stackSize < USLOSS_MIN_STACK) {
        rpt_sim_trap("USLOSS_ContextInit: stackSize < USLOSS_MIN_STACK\n");
    }
    err_return = getcontext(&ctx->context);            
    usloss_sys_assert(err_return != -1, "bad getcontext in USLOSS_ContextInit");
    ctx->context.uc_stack.ss_sp = stack;
    ctx->context.uc_stack.ss_size = stackSize;
    ctx->context.uc_link = NULL;
    makecontext(&ctx->context, launcher, 0);
    ctx->start = pc;
    ctx->initial_psr = psr;
    if (enabled) {
        int_on();
    }
}

/*
 *  The handler for the virtual timer interrupts (among others).
 */
static void sighandler(int sig, siginfo_t *sigstuff, void *oldcontext)
{
    int old_psr = current_psr;
    void *arg;

    /*  We are now in kernel mode - set psr accordingly */

    psr_valid();
    current_psr = USLOSS_PSR_MAGIC | ((current_psr & USLOSS_PSR_CURRENT_MASK) << 2);
    current_psr |= USLOSS_PSR_CURRENT_MODE;
    check_interrupts();
    /*  Switch depending upon what type of signal this is - SIGUSR1 is used
        for system calls, SIG_ALARM is used for devices */
    switch(sig)
    {
      case SIG_ALARM:   /*  Device or clock interrupt - to dispatch routine */
        waiting = 0;    /*  or make this conditional depending on terminal? */
        pclock_ticks++;
        partial_ticks = 0;
        if (syscall_pending) {
            goto done;
        }
        dispatch_int();
        break;
      case SIGUSR1:
        usloss_assert(syscall_pending == 1, "no syscall pending?");
        arg = syscall_arg;
        syscall_pending = 0;
        if (USLOSS_IntVec[USLOSS_SYSCALL_INT] == NULL) {
            rpt_sim_trap("USLOSS_IntVec[USLOSS_SYSCALL_INT] is NULL!\n");
        }
        (*USLOSS_IntVec[USLOSS_SYSCALL_INT])(USLOSS_SYSCALL_INT, arg);
        break;
      case SIGSEGV:
      case SIGBUS:
#ifdef MMU
        USLOSS_MmuHandler(sig, sigstuff, (ucontext_t *) oldcontext);
        break;
#endif
      /* More signals we could catch */
      case SIGFPE:
      default:
        vrpt_cond("Bad signal");
        break;
    }

    /*  Finished with any interrupt handling - reset variables, set up the
        timer for the next interrupt, and go back to the specified context */
done:
    check_interrupts();
    if ((current_psr & ~USLOSS_PSR_MASK) != USLOSS_PSR_MAGIC) {
        usloss_assert(0, "corrupted psr");
    }
    current_psr = old_psr;
#ifdef MMU
    if (mmuInTouch) {
        siglongjmp(mmuTouchBuf, 1);
    }
#endif /* MMU */
}

/*
 * Switches the current context. If the old_context is not NULL the
 * current context is saved there. The current context is then
 * set from the new_context. 
 *
 */
void USLOSS_ContextSwitch(USLOSS_Context *old_context, USLOSS_Context *new_context)
{
    int err_return;
    unsigned int psr;
    int enabled;

    enabled = int_off();
    check_kernel_mode("USLOSS_ContextSwitch");
    check_interrupts();
    psr = current_psr;
    launch_context = new_context;
    if (old_context == NULL) {
        err_return = setcontext(&new_context->context);
    } else {
        check_interrupts();
        err_return = swapcontext(&old_context->context, 
                        &new_context->context);
        current_psr = psr;
    }
    usloss_sys_assert(err_return != -1, "context swap error in "
                      "USLOSS_ContextSwitch)");
    if (enabled) {
        int_on();
    }
}

/*
 *  Interrupt enable/disable/check section
 */

static sigset_t timer_set;

/*
 *  This is called to block delivery of SIG_ALARM to the process, thereby
 *  disabling USLOSS interrupts.
 */

int int_off(void)
{
    int err_return;
    int enabled;
    sigset_t cur_set;

    err_return = sigprocmask(SIG_BLOCK, &timer_set, &cur_set);
    usloss_sys_assert(err_return != -1, "error disabling interrupts");
    enabled = sigismember(&cur_set, SIG_ALARM) ? FALSE : TRUE;
    return enabled;
}

/*
 *  This is called to permit delivery of SIG_ALARM to the process, thereby
 *  enabling USLOSS interrupts.
 *
 *  I modified the timer_set to include SIGUSR1, otherwise when switching to a new context
 *  from an interrupt handler and enabling interrupts SIGUSR1 could be left blocked because
 *  it is in the sa_mask and therefore blocked when the signal handler is invoked. Something
 *  must have changed in the way Linux/Unix handles signal masks. JHH 3/9/09
 */
void int_on(void) 
{
    int err_return;
    sigset_t cur_set;

    err_return = sigprocmask(SIG_UNBLOCK, &timer_set, NULL);
    usloss_sys_assert(err_return != -1, "error enabling interrupts");
    err_return = sigprocmask(SIG_BLOCK, NULL, &cur_set);
    usloss_sys_assert(err_return != -1, "error checking signals");
    usloss_sys_assert(sigismember(&cur_set, SIGUSR1) == 0, "SIGUSR1 is blocked");
}


/*
 *  This routine implements the USLOSS_WaitInt() instruction.  It continually sends
 *  the SIG_ALARM signal until the 'waiting' variable is set to 0 (by the
 *  signal handler). 
 */
void USLOSS_WaitInt(void)
{
    if ((current_psr & USLOSS_PSR_CURRENT_INT) == 0) {
        rpt_sim_trap("USLOSS_WaitInt called with interrupts disabled");
    }
    waiting = 1;
    while (waiting) {
#ifdef VIRTUAL_TIME
        raise(SIG_ALARM);
#else
        pause();
#endif
    }
}

/*
 * System call. The syscall_pending flag is a total hack. Without it
 * a SIG_ALARM signal might show up after the SIGUSR1 signal has been
 * posted but before the signal handler is called. The alarm signal
 * may cause a context switch, causing the wrong process to get the
 * system call signal. I'm not sure why system calls are implemented
 * using signals anyway. jhh 4/5/95
 */
void USLOSS_Syscall(void *arg)
{
    int err_return;
    int enabled;
    sigset_t cur_set;

    if (current_psr & USLOSS_PSR_CURRENT_MODE) {
        USLOSS_Console("FATAL ERROR: Invoking USLOSS_Syscall from kernel mode\n");
        abort();
    }
    /*
     * Make sure SIGUSR1 is not blocked.
     */
    err_return = sigprocmask(SIG_BLOCK, NULL, &cur_set);
    usloss_sys_assert(err_return != -1, "error checking signal mask");
    enabled = sigismember(&cur_set, SIGUSR1) ? FALSE : TRUE;
    if (enabled == FALSE) {
        USLOSS_Console("INTERNAL ERROR: USLOSS_Syscall: invoking raise() with SIGUSR1 blocked.\n");
        abort();
    }
    syscall_pending = 1;
    syscall_arg = arg;
    raise(SIGUSR1);
}

/* ----------------- */

void sig_ints_init(void)
{
    struct sigaction new_act;
    int err_return;

    /*  Set up alarms */
    new_act.sa_sigaction = sighandler;
    new_act.sa_flags = SA_SIGINFO;
    /*
     * We want to contine to receive SIGSEGV signals, even in a signal
     * handler, so don't defer them. The other signals are in the sa_mask
     * so they will be blocked.
     */
    new_act.sa_flags |= SA_NODEFER;
    err_return = sigemptyset(&new_act.sa_mask);
    usloss_sys_assert(err_return != -1, "error creating empty  signal set");
    err_return = sigaddset(&new_act.sa_mask, SIG_ALARM);
    usloss_sys_assert(err_return != -1, "error adding SIG_ALARM to set");
    err_return = sigaddset(&new_act.sa_mask, SIGUSR1);
    usloss_sys_assert(err_return != -1, "error adding SIGUSR1 to set");

    err_return = sigaction(SIG_ALARM, &new_act, &old_actions[SIG_ALARM]);
    usloss_sys_assert(err_return != -1, "error setting up SIG_ALARM action");
    err_return = sigaction(SIGUSR1, &new_act, &old_actions[SIGUSR1]);
    usloss_sys_assert(err_return != -1, "error setting up SIGUSR1 action");
#ifdef MMU
    err_return = sigaction(SIGSEGV, &new_act, &old_actions[SIGSEGV]);
    usloss_sys_assert(err_return != -1, "error setting up SIGSEGV action");
    err_return = sigaction(SIGBUS, &new_act, &old_actions[SIGBUS]);
    usloss_sys_assert(err_return != -1, "error setting up SIGBUS action");
#endif
    /*  Set up the timer_set (used for enabling and disabling interrupts) and
        disable those interrupts */
    err_return = sigemptyset(&timer_set);
    usloss_sys_assert(err_return != -1, "error creating empty timer set");
    err_return = sigaddset(&timer_set, SIG_ALARM);
    usloss_sys_assert(err_return != -1, "error adding SIG_ALARM to timer set");
    err_return = sigaddset(&timer_set, SIGUSR1);
    usloss_sys_assert(err_return != -1, "error adding SIGUSR1 to timer set");
    (void) int_off();
    set_timer();
}


