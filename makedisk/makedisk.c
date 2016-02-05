/*
 * Utility for creating simulated disk for usloss
 */

#include "usloss.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

char	track[USLOSS_DISK_TRACK_SIZE * USLOSS_DISK_SECTOR_SIZE];

int
main(int argc, char **argv)
{
    int 	i, tracks;
    int		fd;
    int		c;
    int		label = 0;
    int		error = 0;
    int		n;
    int		unit;
    char	name[256];

#ifdef NOTDEF
    // don't need disk labels any more
    while((c = getopt(argc, argv, "l")) != EOF) {
#endif /* NOTDEF */
    while((c = getopt(argc, argv, "")) != EOF) {
	switch (c) {
	    case 'l':
		label = 1;
		break;
	    case '?':
		error = 1;
		break;
	}
    }
    if (error) {
	goto usage;
    }
    if (optind == argc) {
	printf("Which unit? ");
	n = scanf("%d", &unit);
	if (n != 1) {
	    goto usage;
	}
	printf("How many tracks? ");
	n = scanf("%d", &tracks);
	if (n != 1) {
	    goto usage;
	}
    } else {
	n = sscanf(argv[optind], "%d", &unit);
	if (n != 1) {
	    goto usage;
	}
	optind++;
	if (optind == (argc-1)) {
	    n = sscanf(argv[optind], "%d", &tracks);
	    if (n != 1) {
		goto usage;
	    }
	} else {
	    printf("How many tracks? ");
	    n = scanf("%d", &tracks);
	    if (n != 1) {
		goto usage;
	    }
	}
    }
    printf("Creating disk file 'disk%d' for unit %d with %d tracks\n", 
	unit, unit, tracks);
    sprintf(name, "disk%d", unit);
    fd = open(name, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
	perror("makedisk can't open disk");
	exit(1);
    }
    memset(track, 0, sizeof(track));
    if (label) {
	n = sprintf(track, "USLOSS Disk\n");
	n += sprintf(&track[n], "Tracks: %d\n", tracks);
	n += sprintf(&track[n], "SectorsPerTrack: %d\n", USLOSS_DISK_TRACK_SIZE);
	n += sprintf(&track[n], "BytesPerSector: %d\n", USLOSS_DISK_SECTOR_SIZE);
	if (n > USLOSS_DISK_SECTOR_SIZE) {
	    fprintf(stderr,"Internal error: label larger than a sector\n");
	}
    }
    for (i = 0; i < tracks; i++) {
	write(fd, track, sizeof(track));
	if ((i == 0) && label) {
	    memset(track, 0, USLOSS_DISK_SECTOR_SIZE);
	}
    }
    close(fd);
    //error = truncate(name, tracks * USLOSS_DISK_TRACK_SIZE * USLOSS_DISK_SECTOR_SIZE);
    if (error) {
	perror("makedisk");
	exit(1);
    }
    exit(0);
usage:
    fprintf(stderr, "Usage: makedisk [unit] [tracks]\n");
    exit(1);

}
