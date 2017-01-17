
#include <stdio.h>
#include "project.h"
#include "globals.h"
#include "usloss.h"
#include "dev_alarm.h"
#include "dev_clock.h"
#include "dev_disk.h"
#include "dev_term.h"

static struct {
    int		device;
    void	*arg;
} dev_event_queue[256];		// Indexed by unsigned char - must be
				// 256 (major changes if not) */

static unsigned char dev_event_ptr;	/*  Index into queue of pending ints */

void (*USLOSS_IntVec[USLOSS_NUM_INTS])(int dev, void *arg);	/*  Interrupt vector table */
     
/*
 *  Initialize USLOSS interrupt processing routines.
 */
dynamic_fun void devices_init(void)
{
    int count;

    /*  Initialize the device event queue */
    for (count = 0; count < 256; count++)
	dev_event_queue[count].device = LOW_PRI_DEV;
    dev_event_ptr = 0;
    /*  Initialize the device status and interrupt vector tables */
    for (count = 0; count < USLOSS_NUM_INTS; count++)
    {
	USLOSS_IntVec[count] = NULL;
    }
}

/*
 *  Schedule an interrupt for a given number of clock ticks (must be < 255)
 *  in the future.  When two interrupts are scheduled for the same tick,
 *  the interrupt with lower priority is assigned to a later tick.
 */
dynamic_fun void schedule_int(int device, void *arg, int future_time)
{
    unsigned char index;	/* index MUST be char for wrap */
    int 	low_pri_dev;
    void 	*low_pri_arg;

    index = ((unsigned char) future_time) + dev_event_ptr;
    do
    {
	/*  Loop until we find a lower priority device than current */
	while(dev_event_queue[index].device <= device)
	    index++;
	/*  Swap the lower with the higher and continue */
	low_pri_dev = dev_event_queue[index].device;
	low_pri_arg = dev_event_queue[index].arg;
	dev_event_queue[index].device = device;
	dev_event_queue[index].arg = arg;
	device = low_pri_dev;
	arg = low_pri_arg;
    }
    while(low_pri_dev != LOW_PRI_DEV);
}

/*
 *  Gets the next event from the queue at interrupt time and performs
 *  all processing needed for this interrupt - calling the device
 *  action routine and the user interrupt handler.
 */
dynamic_fun void dispatch_int(void)
{
    static unsigned int tick = 0;
    int event_device;
    int unit_num = -1;
    void *arg;

    /*  Update and check the 'tick' variable to see if this is a clock
	interrupt */
    tick = ~tick;
    if (tick)
    {
	clock_action();
	if (USLOSS_IntVec[USLOSS_CLOCK_INT] == NULL) {
	    rpt_sim_trap("USLOSS_IntVec[USLOSS_CLOCK_INT] is NULL!\n");
	}
	(*USLOSS_IntVec[USLOSS_CLOCK_INT])(USLOSS_CLOCK_DEV, 0);
	return;
    }

    /*  This is not a clock interrupt - get the next event (from a device) */
    dev_event_ptr++;
    event_device = dev_event_queue[dev_event_ptr].device;
    arg = dev_event_queue[dev_event_ptr].arg;

    /*  Reset the queue entry to the terminal device and perform the
	action for this device */
    dev_event_queue[dev_event_ptr].device = LOW_PRI_DEV;
    switch(event_device)
    {
      case USLOSS_ALARM_DEV:
	unit_num = alarm_action(arg);
	break;
      case USLOSS_DISK_DEV:
	unit_num = disk_action(arg);
	break;
      case USLOSS_TERM_DEV:
	unit_num = term_action(arg);
	break;
      default:
        {
	    char msg[60];

	    sprintf(msg, "illegal device number %d in event queue, index %u",
		event_device, dev_event_ptr);
	    usloss_usr_assert(0, msg);
	}
    }

    /*  If the unit returned from the device action routine is -1, do
	nothing, otherwise call the user interrupt handler */
    if (unit_num != -1)
    {
	USLOSSwaiting = 0;		/*  Even on terminal input?? */
	if (USLOSS_IntVec[event_device] == NULL) {
	    rpt_sim_trap("USLOSS_IntVec contains NULL handle for interrupt.\n");
	}
	(*USLOSS_IntVec[event_device])(event_device, (void *) unit_num);
    }
}

/*
 *  Perform the inp() operation, which returns the status of a device.  We
 *  call on a per-device basis because the device may clear its status when
 *  the USLOSS_DeviceInput() is performed.
 */
int USLOSS_DeviceInput(unsigned int dev, int unit, int *statusPtr)
{
    int result = USLOSS_DEV_INVALID;
    check_kernel_mode("USLOSS_DeviceInput");
    switch(dev)
    {
      case USLOSS_CLOCK_DEV:
	result = clock_get_status(unit, statusPtr);
	break;
      case USLOSS_ALARM_DEV:
	result =  alarm_get_status(unit, statusPtr);
	break;
      case USLOSS_DISK_DEV:
	result =  disk_get_status(unit, statusPtr);
	break;
      case USLOSS_TERM_DEV:
	result =  term_get_status(unit, statusPtr);
	break;
    }
    usloss_sys_assert((result == USLOSS_DEV_OK) || (result == USLOSS_DEV_INVALID),
	"bogus result in USLOSS_DeviceInput");
    return result;
}

/*
 *  Perform the USLOSS_DeviceOutput() operation, which is translated into 
 * a request to a device.
 */
int USLOSS_DeviceOutput(unsigned int dev, int unit, void *arg)
{
    int		result = USLOSS_DEV_ERROR;

    check_kernel_mode("USLOSS_DeviceOutput");
    switch(dev)
    {
      case USLOSS_CLOCK_DEV:
	result = clock_request(unit, arg);
	break;
      case USLOSS_ALARM_DEV:
	result = alarm_request(unit, arg);
	break;
      case USLOSS_DISK_DEV:
	result = disk_request(unit, arg);
	break;
      case USLOSS_TERM_DEV:
	result = term_request(unit, arg);
	break;
    }
    usloss_sys_assert((result == USLOSS_DEV_OK) || (result == USLOSS_DEV_INVALID)
	|| (result == USLOSS_DEV_BUSY),
	"bogus result in USLOSS_DeviceOutput");
    return result;
}

