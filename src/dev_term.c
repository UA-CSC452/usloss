
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "project.h"
#include "globals.h"
#include "dev_term.h"

/*
 * These structures keep track of the status of each terminal. 
 */
typedef struct {
    FILE	*inputPtr;	/* input stream. */
    FILE	*outputPtr;	/* output stream. */
    int		status;		/* its status register. */
    int		control;	/* its control register. */
} TermInfo;

static TermInfo terms[USLOSS_TERM_UNITS];

/* 
 * Handy macros.
 */

#define SET_XMIT_STATUS(status, value)\
    (status) &= ~0xc;\
    (status) |= (((value) & 0x3) << 2);

#define SET_RECV_STATUS(status, value)\
    (status) &= ~0x3;\
    (status) |= ((value) & 0x3);

#define SET_CHAR(status, ch)\
    (status) &= ~0xff00;\
    (status) |= (((ch) & 0xff) << 8);

#ifdef NOTDEF

static char *status2str[] = {"ready", "busy", "error"};

static void
print_status(int status) {
    printf("status: char: %c xmit: %s recv: %s\n", USLOSS_TERM_STAT_CHAR(status),
           status2str[USLOSS_TERM_STAT_XMIT(status)],
           status2str[USLOSS_TERM_STAT_RECV(status)]);
}

static void
print_control(int control) {
    printf("control: char: %c", (control >> 8) & 0xff);
    if (control & 4) {
        printf(", xmit enable");
    }
    if (control & 2) {
        printf(", recv enable");
    }
    if (control & 1) {
        printf(", send char");
    }
    printf("\n");
}

#endif

/* 
 *  Open a file or "/dev/null" if file nonexistent
 */
static FILE *safeopen(char *fname, char *fmode)
{
    FILE *new_file;

    new_file = fopen(fname, fmode);
    if (new_file != 0)
	return new_file;
    new_file = fopen("/dev/null", fmode);
    usloss_sys_assert(new_file != 0, "couldn't safeopen file");
    return new_file;
}

/*
 *	Initialize the terminal device (a single device with four units).
 */
dynamic_dcl void term_init(void)
{
    static char filename[]  = "term_.out";
    int count;

    /* Initialize the state of each terminal. */
    for (count = 0; count < 4; count++)
    {
	terms[count].control = 0;
	terms[count].status = 0;
    }
    /*  Open pseudo-terminal files - output first */
    for (count = 0; count < 4; count++)
    {
	filename[4] = '0' + count;
	terms[count].outputPtr = safeopen(filename, "w");
    }

    /*  Now open the input files */
    strcpy(&filename[6], "in");
    for (count = 0; count < 4; count++)
    {
	filename[4] = '0' + count;
	terms[count].inputPtr = safeopen(filename, "r");
    }
}

/*
 *  Special character input routine for buffered input. If getc()
 *  indicates that EOF has been reached, a read() is attempted to
 *  catch any character that may have been appended since the EOF
 *  was detected. This is a bit of a kludge, but it works.
 */
static int nextchr(FILE *stream)
{
    int c;
    char ch;

    c = getc(stream);
    if (c != EOF) 
	return c;
    c = read(fileno(stream), &ch, 1);
    if (c != 0)
	return ch;
    return EOF;
}

/*
 *  Returns the status of the terminal device
 */
dynamic_dcl int term_get_status(int unit, int *statusPtr)
{

    if ((unit < 0) || (unit > 3)) {
	return USLOSS_DEV_INVALID;
    }
    *statusPtr = terms[unit].status;
    /*
     * Clear the receive side of the terminal.
     */
    SET_RECV_STATUS(terms[unit].status, USLOSS_DEV_READY);
    usloss_sys_assert(USLOSS_TERM_STAT_RECV(terms[unit].status) == USLOSS_DEV_READY, 
	"status botched");
    return USLOSS_DEV_OK;
}

/*
 *  Writes to a terminal's control register. If a character is being 
 *  sent and the device is not busy, then write the character to the file 
 *  and mark the device as busy.
 */
dynamic_dcl int term_request(int unit, void *arg)
{
    int err_return;
    int	ch;
    int req = (int) arg;

    if ((unit < 0) || (unit > 3)) {
	   return USLOSS_DEV_INVALID;
    }
    terms[unit].control = req;
    /*
     * Check to see if we are supposed to send a character.
     */
    if (req & 0x1) {
    	if (USLOSS_TERM_STAT_XMIT(terms[unit].status) == USLOSS_DEV_READY) {
    		ch = (req >> 8) & 0xff;
    		err_return = putc(ch, terms[unit].outputPtr);
    		usloss_sys_assert(err_return != EOF, 
    			"error on putc to terminal device");
    		err_return = fflush(terms[unit].outputPtr);
    		usloss_sys_assert(err_return == 0, 
    			"error on fflush of terminal device");
    		SET_XMIT_STATUS(terms[unit].status, USLOSS_DEV_BUSY);
    	} else if (USLOSS_TERM_STAT_XMIT(terms[unit].status) == USLOSS_DEV_BUSY) {
    	    return USLOSS_DEV_BUSY;
    	}
    }
    return USLOSS_DEV_OK;
}

/*
 *  Perform all actions necessary for reading a character from the terminal
 *  and setting up the device and unit status accordingly.
 */
dynamic_dcl int term_action(void *arg)
{
    static int unit = -1;
    int in_char;
    int result = -1;

    /*  Select the pseudoterminal to read from and get next character */ 
    unit = (unit + 1) % 4;
    //printf("term_action %d\n", unit);
    //print_status(terms[unit].status);
    //print_control(terms[unit].control);

    in_char = nextchr(terms[unit].inputPtr);
    //terms[unit].status = 0;

    /*  If we are not at EOF or the character is not an '@' sign (which
	means pause the input), then set termPtr so subsequent calls
	to term_get_status return the status. */
    if ((in_char != EOF) && ((char) in_char != '@'))
    {
		SET_CHAR(terms[unit].status, in_char);
		SET_RECV_STATUS(terms[unit].status, USLOSS_DEV_BUSY);
		/*
		 * Do not return a unit number if receive interrupts are not
		 * enabled.
		 */
		if (terms[unit].control & 0x2) {
			result = unit;
		}
    }
    else {
    	SET_RECV_STATUS(terms[unit].status, USLOSS_DEV_READY);
    }
    /* 
     * If the xmit side is busy, then we just sent a character. Mark
     * the xmit side as ready.
     */
    if (USLOSS_TERM_STAT_XMIT(terms[unit].status) == USLOSS_DEV_BUSY) {
	   SET_XMIT_STATUS(terms[unit].status, USLOSS_DEV_READY);
       // If xmit interrupt is enabled then generate an interrupt. 
	   if (terms[unit].control & 0x4) {
	       result = unit;
       }
    }
    return result;
}

