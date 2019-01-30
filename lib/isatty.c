/* isatty.c - isatty */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  isatty  -  tests whether a device is a terminal
 *------------------------------------------------------------------------
 *  Usage:
 *	k = isatty(dev);
 *  Returns:
 *      SYSERR if dev is not a valid device number
 *	0 if dev is not a terminal
 *	1 if dev is a terminal
 */
int isatty (int dev)
{
    if (isbaddev(dev)) {
	proctab[currpid].errno = EBADF;
	return SYSERR;
    }

    /*--------------------------------------------------------*/
    /* For now we're just going to do a manual determination  */
    /* of the result. There should be some more automated way */
    /* of achieving this though.                              */
    /*--------------------------------------------------------*/
    if (dev == 0 || dev == 8) return 1;		/* CONSOLE and UE0 */
    return 0;
}
