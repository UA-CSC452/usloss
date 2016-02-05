
#if !defined(_sig_ints_h)
#define _sig_ints_h

#define ALARM_TIME 10000	/*  # of microseconds per clock tick */

dynamic_dcl void set_timer(void);
dynamic_dcl void sig_ints_init(void);
dynamic_dcl int int_off(void);
dynamic_dcl void int_on(void);

#endif	/*  _sig_ints_h */

