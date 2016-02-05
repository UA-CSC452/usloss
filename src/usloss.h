/*
 *  User-visible definitions for usloss. Users of USLOSS are probably
 *  interested in everything in here.
 */

 // DO NOT MODIFY THIS FILE.

#if !defined(_usloss_h)
#define _usloss_h

#include <stdarg.h>
#include <signal.h>
#include <ucontext.h>

typedef struct USLOSS_Context {
    void		(*start)();	/* Starting routine. */
    unsigned int	initial_psr;	/* Initial PSR */
    ucontext_t		context;	/* Internal context state */
} USLOSS_Context;

/*  Function prototypes for USLOSS functions */
extern int		USLOSS_DeviceInput(unsigned int dev, int unit, int *status);
extern int		USLOSS_DeviceOutput(unsigned int dev, int unit, void *arg);
extern void		USLOSS_WaitInt(void);
extern void		USLOSS_Halt(int dumpcore);
extern void		USLOSS_Console(char *string, ...);
extern void		USLOSS_VConsole(char *string, va_list ap);
extern void		USLOSS_Trace(char *string, ...);
extern void		USLOSS_VTrace(char *string, va_list ap);
extern void		USLOSS_ContextInit(USLOSS_Context *state, unsigned int psr,
			    char *stack, int stackSize, void (*func)(void));
extern void		USLOSS_ContextSwitch(USLOSS_Context *old, USLOSS_Context *new);
extern unsigned int	USLOSS_PsrGet(void);
extern void		USLOSS_PsrSet(unsigned int psr);
extern int		USLOSS_Clock(void);
extern void		USLOSS_Syscall(void *arg);

/*
 *  This tells how many slots are in the intvec
 *  USLOSS_NUM_INTS = number of device types +  1 (for syscall interrupt)
 */
#define USLOSS_NUM_INTS	6	/* number of interrupts */

/*
 *  This is the interrupt vector table
 */
extern void (*USLOSS_IntVec[USLOSS_NUM_INTS])(int dev, void *arg);

/* 
 *  These are the values for the individual interrupts
 *  in the interrupt vector.
 */
#define USLOSS_CLOCK_INT	0	/* clock */
#define USLOSS_ALARM_INT	1	/* alarm */
#define USLOSS_DISK_INT		2	/* disk */
#define USLOSS_TERM_INT		3	/* terminal */ 
#define USLOSS_MMU_INT		4	/* MMU */
#define USLOSS_SYSCALL_INT 	5	/* syscall */

#define LOW_PRI_DEV	USLOSS_TERM_INT  /* terminal is lowest priority */


/*
 * These are the different device types. They must be the same values as the
 * interrupts and the USLOSS code conflats the two, but having different macros
 * for devices and interrupts makes the OS code cleaner.
 */

#define USLOSS_CLOCK_DEV 	USLOSS_CLOCK_INT	
#define USLOSS_ALARM_DEV 	USLOSS_ALARM_INT
#define USLOSS_DISK_DEV		USLOSS_DISK_INT
#define USLOSS_TERM_DEV		USLOSS_TERM_INT

/*
 * # of units of each device type
 */

#define USLOSS_CLOCK_UNITS	1
#define USLOSS_ALARM_UNITS	1
#define USLOSS_DISK_UNITS	2
#define USLOSS_TERM_UNITS	4
/*
 * Maximum number of units of any device.
 */

#define USLOSS_MAX_UNITS	4

/*
 *  This is the structure used to send a request to
 *  a device.
 */
typedef struct USLOSS_DeviceRequest
{
	int opr;
	void *reg1;
	void *reg2;
} USLOSS_DeviceRequest;

/*
 *  These are the operations for the disk device
 */
#define USLOSS_DISK_READ	0
#define USLOSS_DISK_WRITE	1
#define USLOSS_DISK_SEEK	2
#define USLOSS_DISK_TRACKS	3

/*
 *  These are the status codes returned by USLOSS_DeviceInput(). In general, 
 *  the status code is in the lower byte of the int returned; the upper
 *  bytes may contain other info. See the documentation for the
 *  specific device for details.
 */
#define USLOSS_DEV_READY	0
#define USLOSS_DEV_BUSY		1
#define USLOSS_DEV_ERROR	2

/* 
 * USLOSS_DeviceOutput() and USLOSS_DeviceInput() will return DEV_OK if their 
 * arguments were valid and the device is ready, DEV_BUSY if the arguments were valid
 * but the device is busy, and DEV_INVALID otherwise. By valid, the device 
 * type and unit must correspond to a device that exists. 
 */

#define USLOSS_DEV_OK		USLOSS_DEV_READY
#define USLOSS_DEV_INVALID	USLOSS_DEV_ERROR

/*
 * These are the fields of the terminal status registers. A call to
 * USLOSS_DeviceInput will return the status register, and you can use these
 * macros to extract the fields. The xmit and recv fields contain the
 * status codes listed above.
 */

#define USLOSS_TERM_STAT_CHAR(status)\
	(((status) >> 8) & 0xff)	/* character received, if any */

#define	USLOSS_TERM_STAT_XMIT(status)\
	(((status) >> 2) & 0x3) 	/* xmit status for unit */

#define	USLOSS_TERM_STAT_RECV(status)\
	((status) & 0x3)		/* recv status for unit */

/*
 * These are the fields of the terminal control registers. You can use
 * these macros to put together a control word to write to the
 * control registers via USLOSS_DeviceOutput.
 */

#define USLOSS_TERM_CTRL_CHAR(ctrl, ch)\
	((ctrl) | (((ch) & 0xff) << 8))/* char to send, if any */

#define	USLOSS_TERM_CTRL_XMIT_INT(ctrl)\
	((ctrl) | 0x4)			/* enable xmit interrupts */

#define	USLOSS_TERM_CTRL_RECV_INT(ctrl)\
	((ctrl) | 0x2)			/* enable recv interrupts */

#define USLOSS_TERM_CTRL_XMIT_CHAR(ctrl)\
	((ctrl) | 0x1)			/* xmit the char in the upper bits */


/*
 *  Size of disk sector (in bytes) and number of sectors in a track
 */
#define USLOSS_DISK_SECTOR_SIZE		512
#define USLOSS_DISK_TRACK_SIZE		16

/*
 * Processor status word (PSR) fields. Current is the current mode
 * and interrupt values, prev are the values prior to the last
 * interrupt. The interrupt handler moves current into prev on an
 * interrupt, and restores current from prev on returning.
 */

#define USLOSS_PSR_CURRENT_MODE 	0x1
#define USLOSS_PSR_CURRENT_INT		0x2
#define USLOSS_PSR_PREV_MODE		0x4
#define USLOSS_PSR_PREV_INT		0x8

#define USLOSS_PSR_CURRENT_MASK	0x3
#define USLOSS_PSR_PREV_MASK		0xc
#define USLOSS_PSR_MASK 		(USLOSS_PSR_CURRENT_MASK | USLOSS_PSR_PREV_MASK)

/*
 * Length of a clock tick.
 */

#define USLOSS_CLOCK_MS	20

/*
 * Minimum stack size. 
 */

#define USLOSS_MIN_STACK (80 * 1024)

/* 
 * Routines that USLOSS invokes for test setup and cleanup. Must be defined by the test code.
 */

extern void setup(void);
extern void cleanup(void);

/*
 * Routines that USLOSS invokes on startup and shutdown. Must be defined by the OS.
 */

extern void startup(void);
extern void finish(void);




/*
 * MMU definitions.
 */

 #define USLOSS_MMU_NUM_TAG	4	/* Maximum number of tags in MMU */

/*
 * Error codes
 */
#define USLOSS_MMU_OK		0	/* Everything hunky-dory */
#define USLOSS_MMU_ERR_OFF	1	/* MMU not enabled */
#define USLOSS_MMU_ERR_ON	2	/* MMU already initialized */
#define USLOSS_MMU_ERR_PAGE	3	/* Invalid page number */
#define USLOSS_MMU_ERR_FRAME	4	/* Invalid frame number */
#define USLOSS_MMU_ERR_PROT	5	/* Invalid protection */
#define USLOSS_MMU_ERR_TAG	6	/* Invalid tag */
#define USLOSS_MMU_ERR_REMAP	7	/* Page already mapped */
#define USLOSS_MMU_ERR_NOMAP	8	/* Page not mapped */
#define USLOSS_MMU_ERR_ACC	9	/* Invalid access bits */
#define USLOSS_MMU_ERR_MAPS	10	/* Too many mappings */

/*
 * Protections
 */
#define USLOSS_MMU_PROT_NONE	0	/* Page cannot be accessed */
#define USLOSS_MMU_PROT_READ	1	/* Page is read-only */
#define USLOSS_MMU_PROT_RW	3	/* Page can be both read and written */

/*
 * Causes
 */
#define USLOSS_MMU_FAULT	1	/* Address was in unmapped page */
#define USLOSS_MMU_ACCESS	2	/* Access type not permitted on page */

/*
 * Access bits
 */
#define USLOSS_MMU_REF		1	/* Page has been referenced */
#define USLOSS_MMU_DIRTY	2	/* Page has been written */

/*
 * Function prototypes for MMU routines. See the MMU documentation.
 */

extern int 	USLOSS_MmuInit(int numMaps, int numPages, int numFrames);
extern void	*USLOSS_MmuRegion(int *numPagesPtr);
extern int	USLOSS_MmuDone(void);
extern int	USLOSS_MmuMap(int tag, int page, int frame, int protection);
extern int	USLOSS_MmuUnmap(int tag, int page);
extern int	USLOSS_MmuGetMap(int tag, int page, int *framePtr, int *protPtr);
extern int	USLOSS_MmuGetCause(void);
extern int	USLOSS_MmuSetAccess(int frame, int access);
extern int	USLOSS_MmuGetAccess(int frame, int *accessPtr);
extern int	USLOSS_MmuSetTag(int tag);
extern int	USLOSS_MmuGetTag(int *tagPtr);
extern int	USLOSS_MmuPageSize(void);
extern int	USLOSS_MmuTouch(void *addr);


#endif	/*  _usloss_h */

