
#if !defined(_devices_h)
#define _devices_h

#include "project.h"
#include "usloss.h"

/*  Variables used by other USLOSS routines */
dynamic_dcl int device_status[USLOSS_NUM_INTS];

/*  Functions used by other USLOSS routines */
dynamic_dcl void devices_init(void);
dynamic_dcl void schedule_int(int device, void *arg, int future_time);
dynamic_dcl void dispatch_int(void);

#endif	/*  _devices_h */

