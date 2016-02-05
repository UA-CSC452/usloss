
#if !defined(_dev_alarm_h)
#define _dev_alarm_h

#include "project.h"
#include "usloss.h"

dynamic_dcl void alarm_init(void);
dynamic_dcl int alarm_get_status(int unit, int *statusPtr);
dynamic_dcl int alarm_request(int unit, void *request);
dynamic_dcl int alarm_action(void *arg);

#endif	/*  _dev_alarm_h */

