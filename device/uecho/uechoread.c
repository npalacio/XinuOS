/* uechoread.c  -  uechoread */
/* This will never read more than a line of data. */

#include <xinu.h>

extern struct uedevice uedev;

/*------------------------------------------------------------------------
 * uechoread  -  Read bytes from a uecho device
 *------------------------------------------------------------------------
 */

devcall	uechoread (
	  struct dentry	*devptr,	/* Entry in device switch table	*/
	  char	*buff,			/* Buffer to hold disk block	*/
	  int32	n			/* number of bytes to read */
	)
{
	int32 i;			/* buffer index */
	int32 c;			/* result of uechoget */

	for(i=0;i<n;i++) {
	    c = uechoget(devptr);

	    if (c == SYSERR)		/* Oops. */
		return SYSERR;

	    if (c == EOF) { 		/* Return # of bytes actually read. */
		if (i > 0)
		    return i;
		return (devcall)EOF;
	    }

	    buff[i] = c;

	    if (c == '\n')		/* Stop after end of line. */
		return (devcall)(i+1);
	}

	return (devcall)n;
}
