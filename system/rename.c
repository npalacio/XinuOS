/* rename.c - rename */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  rename  -  rename a file
 *------------------------------------------------------------------------
 */
syscall	rename(char *old, char *new)
{
    char fullold[NM_MAXLEN];
    char fullnew[NM_MAXLEN];
    struct dentry *devptr;
    int dev, dev2;

    /*------------------------------------------------------------*/
    /* map names through namespace and restrict to single device. */
    /*------------------------------------------------------------*/
    if ( (dev = nammap(old, fullold, NAMESPACE)) == SYSERR)
	return SYSERR;
    if ( (dev2 = nammap(new, fullnew, NAMESPACE)) == SYSERR)
	return SYSERR;
    if (dev != dev2) {		/* both must be on same device */
	proctab[currpid].errno = EXDEV;
	return SYSERR;
    }

    devptr = &devtab[dev];

    return (*devptr->dvcntl)(devptr,F_CTL_RENAME,(int32)fullold,(int32)fullnew);
}

