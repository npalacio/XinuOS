/* rflputc.c - rflputc */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  rflputc  -  Write one character to a remote file
 *------------------------------------------------------------------------
 */
devcall	rflputc(
	struct	dentry	*devptr,	/* Entry in device switch table	*/
	char	ch			/* Character to write		*/
	)
{
#if 0
	/* XXX What's the purpose of rfptr in this code? */
	struct	rflcblk	*rfptr;		/* Pointer to rfl control block	*/

	rfptr = &rfltab[devptr->dvminor];
#endif

	if (rflwrite(devptr, &ch, 1) != 1) {
		return SYSERR;
	}

	return OK;
}
