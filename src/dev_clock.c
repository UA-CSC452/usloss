
#include <stdio.h>
#include "project.h"
#include "globals.h"
#include "dev_clock.h"

/*
 *	Initialize the clock device - nothing to do here, really
 */
dynamic_dcl void clock_init(void)
{
}

/*
 *  Returns the status of the clock device which is the uptime.
 */
dynamic_dcl int clock_get_status(int unit, int *statusPtr)
{
    if (unit != 0) {
	   return USLOSS_DEV_INVALID;
    }
    *statusPtr = USLOSSClock();
    return USLOSS_DEV_OK;
}

/*
 *  There are really no requests that can be made of the clock.....
 */
dynamic_dcl int clock_request(int unit, void *arg)
{
    if (unit != 0) {
	return USLOSS_DEV_INVALID;
    }
    return USLOSS_DEV_OK;
}

/*
 *  And, it doesn't really do much here, either....
 */
dynamic_dcl int clock_action(void)
{
    return 0;
}

