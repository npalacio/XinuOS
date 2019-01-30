/* rflgetc.c - rflgetc */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  rflgetc  -  Read one character from a remote file
 *------------------------------------------------------------------------
 */
devcall	rflgetc(
	struct	dentry	*devptr		/* Entry in device switch table	*/
	)
{
	char	ch;			/* Character to read		*/
	int32	retval;			/* Return value			*/

	retval = rflread(devptr, &ch, 1);

	if (retval == 0) {
		return (devcall)EOF;
	}

	if (retval == SYSERR) {
		return SYSERR;
	}

	return (devcall)ch;
}
