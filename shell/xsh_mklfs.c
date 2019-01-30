/* xsh_mklfs.c - xsh_mklfs */

#include <xinu.h>
#include <ramdisk.h>

struct lfdata Lf_data;

/*------------------------------------------------------------------------
 * xsh_mklfs - Create an empty local file system
 *------------------------------------------------------------------------
 * With no options, there will be 20 index blocks (enough for each of the
 * 20 files to have a maximum size of 8192), and RM_BLKSIZ * RM_BLKS as
 * the total disk size.
 * The -i# option can be used to specify the number of index blocks, where
 *     # must be at least 2 but no more than rdsize;
 * The -n# option can be used to specify the number of total blocks in the
 *     file system; it must be larger than rdndx;
 */
shellcmd xsh_mklfs(int nargs, char *args[])
{
    status k;				/* result of lfscreate */
    int32 force;			/* non-zero to force creation */
    int32 rdndx, rdblocks, rdsize;
    char *endptr;			/* ptr to end of converted string */
    long v;				/* converted string's value */

    if (nargs == 2 && !strcmp(args[1],"--help")) {
	printf("mklfs creates an empty local filesystem on RAM disk RAM0.\n");
	printf("Options:\n");
	printf("    --help     display this help.\n");
	printf("    -f         force creation even if filesystem already"
                               " exists\n");
	printf("    -i#        allocate # index blocks (default is 20)\n");
	printf("    -n#        use # blocks for filesystem (default is %d)\n",
	    RM_BLKS);
	printf("The number of index blocks must be at least 2, and must be\n");
	printf("smaller than the total number of blocks in the filesystem.\n");
	return 0;
    }
	

    rdndx = 20;				/* default number of index blocks */
    rdblocks = RM_BLKS;			/* # of blocks in RAM disk */

    force = 0;

    while (nargs > 1 && args[1][0] == '-') {

	if (args[1][1] == 'i') {
	    v = strtol(args[1]+2, &endptr, 10);
	    if (v == 0 || *endptr != '\0') {
		printf("Bad number of index blocks.\n");
		return 0;
	    }
	    rdndx = v;
	    nargs--;
	    args++;
	    continue;
	}

	if (args[1][1] == 'n') {
	    v = strtol(args[1]+2, &endptr, 10);
	    if (v == 0 || *endptr != '\0') {
		printf("Bad number of file system blocks.\n");
		return 0;
	    }
	    rdsize = v;
	    nargs--;
	    args++;
	    continue;
	}

	if (!strcmp(args[1],"-f")) {
	    force = 1;
	    nargs--;
	    args++;
	    continue;
	}

	printf("Unknown option %s\n", args[1]);
	return 0;
    }

    if (Lf_data.lf_dirpresent == TRUE) {
	if (!force) {
	    printf("Filesystem already exists and will not be recreated.\n");
	    printf("Use the -f option to force filesystem recreation.\n");
	    return 0;
	}
    }

    if (rdblocks <= rdndx || rdndx < 2) {
	printf("Bad number of index blocks or file system blocks.\n");
	return 0;
    }
    rdsize = RM_BLKSIZ * rdblocks;	/* total disk size, in bytes */

    k = lfscreate(RAM0, rdndx, rdsize);
    if (k != OK)
	printf("Error: lfscreate failed.\n");

    return 0;
}
