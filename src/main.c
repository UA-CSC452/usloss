
#include <stdlib.h>
#include <getopt.h>
#include "project.h"
#include "usloss.h"
#include "main.h"
#include "globals.h"
#include "dev_alarm.h"
#include "dev_clock.h"
#include "dev_disk.h"
#include "dev_term.h"
#include "devices.h"
#include "sig_ints.h"

static USLOSS_Context startup_context;
dynamic_def(USLOSS_Context finish_context);
dynamic_def(int finish_status);

static char stack[USLOSS_MIN_STACK];
static int gargc;
static char **gargv;

static void starter(void) {
    startup(gargc, gargv);
    rpt_sim_trap("startup returned!\n");
}

static void print_options()
{
    printf("USLOSS Options:\n");
    printf("  -h, --help               Print list of options and exit.\n");
    printf("  -r, --real-time          Set USLOSS to use real time. This is the default mode.\n");
    printf("  -R, --virtual-time       Set USLOSS to use virtual time.\n");
    printf("  -v, --verbose            Increase the verbosity level of USLOSS. The verbosity level\n");
    printf("                           is equal to the number of times this option is set.\n");
    printf("                           LEVELS:\n");
    printf("                           1 -- Context Creation\n");
    printf("                           2 -- Context Switches\n");
    printf("                           3 -- All interrupts\n");
    printf("                           4 -- Change in PSR\n");
}

// global flags
int verbosity, virtual_time, SIG_ALARM;

int main(int argc, char **argv)
{
    // Parse args
    verbosity = 0;
    virtual_time = FALSE;
    int opt;
    struct option longopt[] = {
        {"verbose", no_argument, NULL, 'v'},
        {"real-time", no_argument, NULL, 'r'},
        {"virtual-time", no_argument, NULL, 'R'},
        {"help", no_argument, NULL, 'h'}
    };
    while ((opt = getopt_long(argc, argv, "vrRh", longopt, NULL)) != -1) {
        switch(opt) {
            case 'v':
                verbosity++;
                break;
            case 'r':
                virtual_time = FALSE;
                break;
            case 'R':
                virtual_time = TRUE;
                break;
            case 'h':
                print_options();
                return 0;
        }
    }

    // SIG_ALARM is now defined at runtime
    SIG_ALARM = virtual_time ? SIGVTALRM : SIGALRM;

    
    unsigned int psr;
    test_setup(argc, argv);
    /*  Call the per-module initialization routines */
    globals_init();
    devices_init();
    alarm_init();
    clock_init();
    disk_init();
    term_init();
    sig_ints_init();	/*  Must disable interrupts */

    gargc = argc - optind;
    gargv = &argv[optind];
    /*  Set up the initial context that runs the user's startup code */
    getcontext(&startup_context.context);
    startup_context.context.uc_stack.ss_sp = stack;
    startup_context.context.uc_stack.ss_size = sizeof(stack);
    startup_context.context.uc_link = &finish_context.context;
    startup_context.context.uc_link = NULL;
    makecontext(&startup_context.context, (FN_CAST) starter, 0);

    /*  Turn on the timer and start running (user must unblock SIG_ALARM via
	the int_disable() function */
    set_timer();
    psr = current_psr;
    swapcontext(&finish_context.context, &startup_context.context);

    /*  Finished from swapcontext() - user has called USLOSS_Halt.  We will call
	their finish() routine and exit */
    current_psr = psr;
    finish(argc, argv);
    test_cleanup(argc, argv);
    exit(finish_status);
}

