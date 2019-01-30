/* uechoclose.c  -  uechoclose */

#include <xinu.h>

/*------------------------------------------------------------------------
 * uechoclose  -  Close a uecho device
 *------------------------------------------------------------------------
 */

extern struct uedevice uedev;

devcall	uechoclose (
	  struct dentry	*devptr		/* Entry in device switch table	*/
	)
{
/* XXX IGNORE CLOSE REQUESTS XXX */
	return (devcall)OK;

	/* If the device isn't open, return SYSERR */
	if (uedev.ueisopen == 0)
	    return SYSERR;

	/* Should we gobble up any remaining packets, discarding them? */
	/* Yes, we should. */

	uedev.ueisopen = 0;
	uedev.reg = 0;
	udp_release(uedev.slot);	/* release claim on UDP packets */

	return (devcall)OK;
}
