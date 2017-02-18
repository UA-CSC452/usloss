/*
 * udisk.c
 * Library for creating simulated disk for USLOSS.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/param.h>
#include "usloss.h"
#include "libdisk.h"


char	track[USLOSS_DISK_TRACK_SIZE * USLOSS_DISK_SECTOR_SIZE];

/*
 * Disk_Create
 *
 * Create and format a USLOSS disk.
 *
 * dir: directory in which to create disk file. If NULL the current working directory is used.
 * unit: unit number of the disk. The disk file will be named "diskN" where N is the unit.
 * tracks: # of tracks the disk contains.
 *
 * Returns: 0 on success, 1 otherwise
 */

int
Disk_Create(char *dir, unsigned int unit, unsigned int tracks) 
{
    int     result = 1;
    char    name[MAXPATHLEN];
    char    track[USLOSS_DISK_TRACK_SIZE * USLOSS_DISK_SECTOR_SIZE];
    int     fd = -1;
    int     n;

    if (dir == NULL) {
        dir = ".";
    }
    snprintf(name, sizeof(name), "%s/disk%d", dir, unit);
    fd = open(name, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
	   perror("unable to open disk file");
       goto done;
    }
    memset(track, 0, sizeof(track));
    for (int i = 0; i < tracks; i++) {
	    n = write(fd, track, sizeof(track));
        if (n != sizeof(track)) {
            perror("unable to write to disk file");
            goto done;
        }
    }
    result = 0;
done:
    if (fd >= 0) {
        close(fd);
    }
    return result;
}
