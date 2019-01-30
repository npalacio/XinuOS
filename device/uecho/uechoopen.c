/* uechoopen.c  -  uechoopen */

#include <xinu.h>

/*------------------------------------------------------------------------
 * uechoopen  -  Open a uecho "device"
 *------------------------------------------------------------------------
 */

extern struct uedevice uedev;

devcall	uechoopen (
	 struct	dentry	*devptr,	/* Entry in device switch table	*/
	 char	*name,			/* Unused for a uecho device	*/
	 char	*mode			/* Unused for a uecho device	*/
	)
{
	char buff[10];			/* buffer for 0-length packet */
	int32 msglen;			/* length of message in packet */
	int32 retval;			/* value returned by udp_register */

	/*-----------------------------------------*/
	/* Register the server port, if necessary. */
	/*-----------------------------------------*/
	if (!uedev.reg) {
	    if ((retval = udp_register(0, 0, UELPORT)) == SYSERR)
		return (devcall)SYSERR;
	    uedev.reg = 1;
	    uedev.slot = retval;
	    uedev.ueisopen = 0;
	}

	if (uedev.ueisopen)		/* device open? return SYSERR */
	    return (devcall)SYSERR;

	/*-----------------------------------------------------------------*/
	/* Wait for a 0-length packet from a client, saving client's addr. */
	/*-----------------------------------------------------------------*/
	for(;;) {
	    msglen = udp_recvaddr(uedev.slot,	/* slot in the UDP table */
		&uedev.remip, &uedev.remport,	/* client's IP addr & port # */
		buff,				/* buffer (really don't care) */
		10,				/* buffer length */
		UE0TIMEOUT);			/* timeout, in msec */
	    if (msglen == 0)
		break;
	    if (msglen == TIMEOUT)		/* on timeout, try again */
		continue;
	    if (msglen == SYSERR)
		return SYSERR;
	    /*-------------------------------------------------------------*/
	    /* Good packets with non-zero lengths are ignored if not open. */
	    /*-------------------------------------------------------------*/
	}
	
	uedev.ueisopen = 1;		/* mark device open */
	uedev.cldisc = 0;		/* client hasn't disconnected */

	return devptr->dvnum;
}
