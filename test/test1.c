#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "usloss.h"

char	buffers[4][256];
int	in[4];
int	out[4];
int	active[4];
int	controls[4];

void
dummy_handler(type, unit)
    int		type;
    int		unit;
{
    fprintf(stderr, "Interrupt: %d\n", type);
    return;
}

void
term_handler(type, unit)
    int		type;
    int		unit;
{
    int		result;
    int		status;
    int		ch;
    int		i;

    active[unit] = 1;
    result = device_input(TERM_DEV, unit, &status);
    console("term_handler %d (0x%x)\n", unit, status);
#ifdef NOTDEF
    assert(result == DEV_OK);
    for (i = 0; i < 4; i++) {
	int	tmp;
	result = device_input(TERM_DEV, i, &tmp);
	assert(result == DEV_OK);
	console("\t%d 0x%x\n", i, tmp);
    }
#endif
    if (TERM_STAT_RECV(status) == DEV_BUSY) {
	ch = TERM_STAT_CHAR(status);
	buffers[unit][in[unit]] = ch;
	in[unit]++;
	if (ch == '\n') {
	    printf("Terminal %d: ", unit);
	    printf(buffers[unit]);
	}
    }
    if ((TERM_STAT_XMIT(status) == DEV_READY) &&
	    buffers[unit][out[unit]] != '\0') {
	int	control;
	i = (unit + 1) % 4;
	control = controls[i];
	control = TERM_CTRL_CHAR(control, buffers[unit][out[unit]]);
	control = TERM_CTRL_XMIT_CHAR(control);
	console("Writing %d 0x%x\n", i, control);
	result = device_output(TERM_DEV, i, control);
	assert(result == DEV_OK);
	result = device_input(TERM_DEV, i, &status);
	assert(TERM_STAT_XMIT(status) == DEV_BUSY);
	control = TERM_CTRL_CHAR(control, 'X');
	result = device_output(TERM_DEV, i, control);
	assert(result == DEV_OK);
	out[unit]++;
    }
    return;
}


void 
startup()
{
    int		i;

    for (i = 0; i < NUM_INT; i++) {
	int_vec[i] = dummy_handler;
    }

    for (i = 0; i < 4; i++) {
	active[i] = 0;
	in[i] = 0;
	out[i] = 0;
	memset(buffers[i], 0, 256);
    }
    int_vec[TERM_DEV] = term_handler;
#ifdef NOTDEF
    for (i = 0; i < 4; i++) {
	controls[i] = 0;
	controls[i] = TERM_CTRL_RECV_INT(controls[i]);
	controls[i] = TERM_CTRL_XMIT_INT(controls[i]);
	device_output(TERM_DEV, i, controls[i]);
    }
#endif
    int_enable();
    while(1) {
	waitint();
    }
}

void
finish()
{
    return;
}
