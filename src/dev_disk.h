
#if !defined(_dev_disk_h)
#define _dev_disk_h

#include "project.h"
#include "usloss.h"

dynamic_dcl void disk_init(void);
dynamic_dcl int disk_get_status(int unit, int *status);
dynamic_dcl int disk_request(int unit, void *request);
dynamic_dcl int disk_action(void *arg);

#endif	/*  _dev_disk_h */

