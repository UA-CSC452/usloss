
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "project.h"
#include "globals.h"
#include "usloss.h"
#include "dev_disk.h"
#include "devices.h"

typedef struct {
    int				fd;		// Open fd for disk file. 
    int				tracks;		// # tracks in the disk.
    int				currentTrack;	// head position
    int				status;		// Disk's status
    USLOSS_DeviceRequest	request;	// Current request
} DiskInfo;

static DiskInfo		disks[USLOSS_DISK_UNITS];

/*
 *  Initialize all disk handling code.
 */
dynamic_fun void disk_init(void)
{
    struct stat inode;
    int 	i;
    char	name[256];

    for (i = 0; i < USLOSS_DISK_UNITS; i++) {
	sprintf(name, "disk%d", i);
	disks[i].fd = open(name, O_RDWR, 0);
	if (disks[i].fd != -1) {
	    /*  Figure out how may tracks it has - check for errors */
	    usloss_sys_assert(fstat(disks[i].fd, &inode) == 0,
			  "Error in fstat() on disk file");
	    if (inode.st_size % (USLOSS_DISK_TRACK_SIZE * USLOSS_DISK_SECTOR_SIZE) != 0) {
		USLOSS_Console("Disk %s has an incomplete last track\n", name);
		close(disks[i].fd);
		disks[i].fd = -1;
	    }
	    disks[i].tracks = inode.st_size / 
		(USLOSS_DISK_TRACK_SIZE * USLOSS_DISK_SECTOR_SIZE);
	    disks[i].currentTrack = 0;
	    disks[i].status = USLOSS_DEV_READY;
	}
    }
}

/*
 *  Returns the current device status of the disk.  Resets the status to
 *  DEV_READY if the last I/O operation resulted in an error.
 */
dynamic_fun int disk_get_status(int unit, int *statusPtr)
{
    if ((unit < 0) || (unit >= USLOSS_DISK_UNITS) || (disks[unit].fd == -1)) {
	return USLOSS_DEV_INVALID;
    }
    *statusPtr = disks[unit].status;
    if (*statusPtr == USLOSS_DEV_ERROR) {
	disks[unit].status = USLOSS_DEV_READY;
    }
    return USLOSS_DEV_OK;
}

/*
 *  Handles requests to the disk device (via the outp() instruction).
 */
dynamic_fun int disk_request(int unit, void *arg)
{
    int rc;
    int delay;
    USLOSS_DeviceRequest *request = (USLOSS_DeviceRequest *) arg;

    if ((unit < 0) || (unit >= USLOSS_DISK_UNITS) || (disks[unit].fd == -1)) {
	rc = USLOSS_DEV_INVALID;
	goto done;
    }
    /*  Check if a request is already pending - if so, do nothing, else
	indicate a pending request */
    if (disks[unit].status == USLOSS_DEV_BUSY) {
	rc = USLOSS_DEV_BUSY;
	goto done;
    }
    disks[unit].status = USLOSS_DEV_BUSY;

    /*  Store the new request data, calculate
	the delay to fulfill the request, and schedule the interrupt */
    memcpy(&disks[unit].request, request, sizeof(*request));
    /* 
     * A disk access should take 30ms (3 ticks), tops.
     */
    if (request -> opr == USLOSS_DISK_SEEK)
	delay = 1 + (abs((disks[unit].currentTrack) - 
	    ((int) request -> reg1)) % 10);
    else 
	delay = 1;
    if (delay > 3)
	delay = 3;
    schedule_int(USLOSS_DISK_INT, (void *) unit, delay);
    rc = USLOSS_DEV_OK;
done:
    return rc;
}

/*
 *  This routine performs the actual I/O actions. It is called just before
 *  the interrupt signalling I/O completion is sent. Note that the virtual
 *  timer is off while the Unix kernel calls are made, making the I/O
 *  operations appear to occur instantaneously.  Impossible requests cause
 *  the device status to be set to DEV_ERROR. The number of sectors per
 *  track (DISK_TRACK_SIZE) is known at compile time, 
 *  while the number of tracks on the disk (disk_tracks) is determined 
 *  at startup time.
 */
dynamic_fun int disk_action(void *arg)
{
    int status = USLOSS_DEV_READY;
    long seek_loc;
    int err_return;
    int unit = (int) arg;
    USLOSS_DeviceRequest *request;

    usloss_sys_assert((unit >= 0) && (unit < USLOSS_DISK_UNITS), 
	"invalid disk unit in disk_action");
    request = &disks[unit].request;

    switch(request->opr)
    {
      case USLOSS_DISK_SEEK:
	if ((((int) request->reg1) >= disks[unit].tracks) ||
	    (((int) request->reg1) < 0))
	    status = USLOSS_DEV_ERROR;
	else
	    disks[unit].currentTrack = (int) request->reg1;
	break;
      case USLOSS_DISK_READ:
      case USLOSS_DISK_WRITE:
	if (((int)request->reg1) >= USLOSS_DISK_TRACK_SIZE)
	    status = USLOSS_DEV_ERROR;
	else
	{
	    seek_loc = ((disks[unit].currentTrack * USLOSS_DISK_TRACK_SIZE) + 
			((int)request->reg1)) * USLOSS_DISK_SECTOR_SIZE;
	    err_return = lseek(disks[unit].fd, seek_loc, 0);
	    usloss_sys_assert(err_return != -1, "error seeking in disk file");
	    if (request->opr == USLOSS_DISK_WRITE)
	    {
		err_return = write(disks[unit].fd, request->reg2,
				   USLOSS_DISK_SECTOR_SIZE);
		usloss_sys_assert(err_return != -1, 
		    "error writing to disk file");
	    }
	    else
	    {
		err_return = read(disks[unit].fd, (void *) request->reg2,
				  USLOSS_DISK_SECTOR_SIZE);
		usloss_sys_assert(err_return == USLOSS_DISK_SECTOR_SIZE, 
		    "error reading from disk file");
	    }
	}
	break;
      case USLOSS_DISK_TRACKS:
	*((int *) request->reg1) = disks[unit].tracks;
	break;
      default:
	usloss_usr_assert(0, "Illegal disk request operation");
	break;
    }
    disks[unit].status = status;
    return unit;
}

