
#if !defined(_dev_clock_h)
#define _dev_clock_h

#include "project.h"
#include "usloss.h"

dynamic_dcl void clock_init(void);
dynamic_dcl int clock_get_status(int unit, int *status);
dynamic_dcl int clock_request(int unit, void *arg);
dynamic_dcl int clock_action(void);

#endif	/*  _dev_clock_h */

