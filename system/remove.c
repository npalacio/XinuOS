/* remove.c - remove */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  remove  -  remove a file given its name (key is optional)
 *------------------------------------------------------------------------
 */
syscall remove(char *name, uint32 key)
{
        char    fullnam[NM_MAXLEN];
        struct  dentry   *devptr;
        int     dev;

        if ( (dev=nammap(name, fullnam, NAMESPACE)) == SYSERR)
	    return SYSERR ;
        devptr = &devtab[dev];
        return(*devptr->dvcntl)(devptr, F_CTL_DEL, (int32)fullnam, key);
}
