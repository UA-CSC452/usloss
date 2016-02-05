
#if !defined(_dev_term_h)
#define _dev_term_h

#include "project.h"
#include "usloss.h"

dynamic_dcl void term_init(void);
dynamic_dcl int term_get_status(int unit, int *status);
dynamic_dcl int term_request(int unit, void *arg);
dynamic_dcl int term_action(void *arg);

#endif	/*  _dev_term_h */

