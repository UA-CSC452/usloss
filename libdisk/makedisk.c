/*
 * Utility for creating simulated disk for usloss
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include "usloss.h"
#include "libdisk.h"

char	track[USLOSS_DISK_TRACK_SIZE * USLOSS_DISK_SECTOR_SIZE];

int
main(int argc, char **argv)
{
    int 	i, tracks;
    int		fd;
    int		c;
    int		n;
    int		unit;
    int     rc;
    int     result = 1;
    int     error = 0;

    while((c = getopt(argc, argv, "")) != EOF) {
    	switch (c) {
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
    rc = Disk_Create(NULL, unit, tracks);
    if (rc != 0) {
        printf("Error creating disk\n");
        goto done;
    }
    result = 0;
done:
    return result;
usage:
    fprintf(stderr, "Usage: makedisk [unit] [tracks]\n");
    return 1;

}
