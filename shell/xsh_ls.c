/* xsh_ls.c - xsh_ls */

#include <xinu.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct lfdata Lf_data;

/*------------------------------------------------------------------------
 * xsh_ls - List the directory entries in a local filesystem.
 *------------------------------------------------------------------------
 * This is the first version of this program. We'll only list the entries
 * in a local filesystem on the ram disk device.
 *
 * New stuff and ideas.
 * (1) Perhaps much later (or never), provide matching of reg. expr. args to
 *     filenames to list only those files of interest (e.g. ls *.c).
 */

struct nment {
    char name[LF_NAME_LEN];	/* file name */
    int size;			/* file size, in bytes */
    int ib;			/* first index block number */
};

/*-------------------------------*/
/* Comparison function for qsort */
/*-------------------------------*/
static int kompar(const void *p1, const void *p2)
{
    struct nment *s1 = (struct nment *)p1;
    struct nment *s2 = (struct nment *)p2;
    int k;

    k = strcmp(s1->name, s2->name);
    return k;
}

shellcmd xsh_ls(int nargs, char *args[])
{
    int i;			/* index to directory */
    int j;			/* index to filename */
    int len;			/* length of filename */
    devcall k;			/* returned value from lfscontrol */
    struct ldentry dent;	/* a returned directory entry     */
    int aopt;			/* "raw" output option */
    int vopt;			/* "verbose" option (for block stats) */
    struct lfdir *dirptr;	/* pointer to the LFS in-memory directory */
    struct lflcblk *lfptr;	/* pointer to open file table entry */
    char *p;			/* pointer to a filename character */

    struct nment nm[LF_NUM_DIR_ENT];
    int nnm;			/* number of entries in nm */

    if (nargs == 2 && !strcmp(args[1],"--help")) {
	printf("List the contents of the local filesystem directory.\n");
	printf("With no options, files are displayed in sorted order.\n");
	printf("\n");
	printf("Options:\n");
	printf("-v   Display index & data block info, open files\n");
	printf("-a   Display raw entries for all directory slots\n");
	return 0;
    }

    vopt = 0;
    aopt = 0;
    while (nargs > 1 && args[1][0] == '-') {
	if (!strcmp(args[1],"-v")) {
	    vopt = 1;
	    nargs--;
	    args++;
	    continue;
	}
	if (!strcmp(args[1],"-a")) {
	    aopt = 1;
	    nargs--;
	    args++;
	    continue;
	}
	printf("Unknown option %s\n", args[1]);
	return 0;
    }

    if (nargs > 1) {
	printf("Unknown argument(s).\n");
	return 0;
    }

    /*-----------------------------------*/
    /* Verify a local filesystem exists. */
    /*-----------------------------------*/
    k = control(LFILESYS,LF_CTL_EXIST,0,0);
    if (k == SYSERR) {
	printf("No local filesystem.\n");
	return 0;
    }

    if (vopt) {
	dirptr = &Lf_data.lf_dir;
	printf("# files in directory = %d\n\n", dirptr->lfd_nfiles);

	/*------------------------*/
	/* Display open file info */
	/*------------------------*/
	printf("Open files:\n");
	for(i=0; i<Nlfl; i++) {		/* scan file pseudo-devices */
	    lfptr = &lfltab[i];
	    if (lfptr->lfstate != LF_FREE) {
		p = lfptr->lfname;
		while (*p != NULLCH) {
		    printf("%c",*p);
		    p++;
		}
		printf("\n");
	    }
	}
	printf("-----\n\nFile directory contents:\n");
    }

    nnm = 0;
    for(i=0;i<LF_NUM_DIR_ENT;i++) {
	k = control(LFILESYS,LF_CTL_GETDE,i,(int)&dent);
	if (k == SYSERR) {
	    if (proctab[currpid].errno == ENOENT)	/* end of dir? */
		break;
	    printf("Error accessing directory entry %d\n", i);
	    return 0;
	}
	if (aopt)
	    printf("%2d: ", i);
	len = 0;
	for (len=0;len<LF_NAME_LEN;len++) {
	    nm[nnm].name[len] = dent.ld_name[len];
	    if (dent.ld_name[len] == NULLCH)
		break;
	}
	if (len == 0) {			/* unused entry? */
	    if (aopt)
		printf("<unused>\n");
	    continue;
	}
	nm[nnm].size = dent.ld_size;
	nm[nnm].ib = dent.ld_ilist;
	nnm++;
	
	if (aopt) {
	    for(j=0;j<len;j++)			/* display name */
		printf("%c",dent.ld_name[j]);
	    for(;j<LF_NAME_LEN+4;j++)		/* space to size field */
		printf("%c",' ');
	    printf("%6d",dent.ld_size);		/* display file size */
	    printf("  (ib=%d)", dent.ld_ilist);	/* display first iblock ID */
	    printf("\n");
	}
    }

    if (aopt == 0) {
	qsort(&nm, nnm, sizeof(struct nment), kompar);
	for(i=0;i<nnm;i++) {
	    printf("%s", nm[i].name);
	    for(j=strlen(nm[i].name);j<LF_NAME_LEN+4;j++)
		printf("%c",' ');
	    printf("%6d",nm[i].size);		/* display file size */
	    printf("  (ib=%d)\n", nm[i].ib);	/* display first iblock ID */
	}
    }

    return 0;
}
