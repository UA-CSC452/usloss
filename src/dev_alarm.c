
#include <stdio.h>
#include "project.h"
#include "globals.h"
#include "dev_alarm.h"
#include "devices.h"

static int armed = 0;

/*
 *	Initialize the alarm device - nothing to do here, really
 */
dynamic_dcl void alarm_init(void)
{
}

/*
 *  Returns the status of the alarm device
 */
dynamic_dcl int alarm_get_status(int unit, int *statusPtr)
{
    if (unit != 0) 
	return USLOSS_DEV_INVALID;
    if (armed) {
	*statusPtr = USLOSS_DEV_BUSY;
    } else {
	*statusPtr = USLOSS_DEV_READY;
    }
    return USLOSS_DEV_OK;
}

/*
 *  
 */
dynamic_dcl int alarm_request(int unit, void *arg)
{
    int time = (int) arg;

    if (unit != 0) {
	return USLOSS_DEV_INVALID;
    }
    armed = 1;
    schedule_int(USLOSS_ALARM_INT, NULL, time);
    return USLOSS_DEV_OK;
}

/*
 *  
 */
dynamic_dcl int alarm_action(void *arg)
{
    armed = 0;
    return 0;
}

