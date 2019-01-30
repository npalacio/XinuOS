/* uechogetc.c  -  uechogetc */

#include <xinu.h>

/*------------------------------------------------------------------------
 * uechoget  -  read a character from a uecho device
 *------------------------------------------------------------------------
 */

extern struct uedevice uedev;

/*---------------------------------------------------------------*/
/* This is somewhat crude. When the "connection" is terminated,  */
/* we perform the actions that would normally be associated with */
/* closing the device here.                                      */
/*---------------------------------------------------------------*/
static void doclose(void)
{
    uedev.ueisopen = 0;			/* mark the device as closed */
    uedev.reg = 0;			/* UDP port is unregistered */
    udp_release(uedev.slot);		/* release claim on UDP packets */
}

devcall	uechoget (
	  struct dentry	*devptr	/* Entry in device switch table	*/
	)
{
	unsigned char rc;
	char buff[1000];		/* buffer for UDP packet */
	int32 msglen;			/* length of message in packet */
	int32 i;			/* index to char in message */

	uint32 remip;			/* IP address of sender */
	uint16 remport;			/* port number of sender */

	uint16 obx;			/* index of char slot at end */

	if (!uedev.ueisopen)		/* device not open? return SYSERR */
	    return (devcall)SYSERR;

	if (uedev.cldisc) {		/* has remote client disconnected? */
	    if (uedev.isz > 0) {	    /* if data left, get a byte of it */
		rc = uedev.ibuff[uedev.ibx];
		uedev.ibx = (uedev.ibx + 1) % UEBUFSZ;
		uedev.isz--;
		if (rc == '\r')
		    rc = '\n';
		return (devcall)rc;
	    } else {			    /* otherwise report end of file */
		doclose();
		return (devcall)EOF;
	    }
	}

	while (uedev.nlines == 0) {	/* if no complete lines in ibuff */
	    /*----------------------------------*/
	    /* Wait for a packet from a client. */
	    /*----------------------------------*/
	    obx = (uedev.ibx + uedev.isz) % UEBUFSZ;
	    msglen = udp_recvaddr(uedev.slot,	/* UDP table slot */
		&remip, &remport,			/* client's addr */
		buff,				/* buffer */
		1000,				/* buffer length */
		UE0TIMEOUT);			/* timeout */

	    if (msglen == TIMEOUT)			/* timeout, repeat */
		continue;

	    if (msglen == SYSERR)			/* bad... very bad */
		return SYSERR;

	    /*---------------------------------------*/
	    /* Verify packet is from correct sender. */
	    /*---------------------------------------*/
	    if (uedev.remip != remip)		/* wrong sender IP */
		continue;
	    if (uedev.remport != remport)		/* wrong sender port */
		continue;

	    if (msglen == 0) {		    /* end this connection */
		uedev.cldisc = 1;
		if (uedev.isz > 0) {	    /* if data left, get a byte of it */
		    rc = uedev.ibuff[uedev.ibx];
		    uedev.ibx = (uedev.ibx + 1) % UEBUFSZ;
		    uedev.isz--;
		    if (rc == '\r')
			rc = '\n';
		    return (devcall)rc;
		} else {		    /* otherwise report end of file */
		    doclose();
		    return (devcall)EOF;
		}
	    }

	    /*---------------------------------------------*/
	    /* Later we'll check for control packets here. */
	    /*---------------------------------------------*/

	    for(i=0;i<msglen;i++) {		    /* process each msg char */
		/*---------------------------*/
		/* Check for buffer overflow */
		/*---------------------------*/
		if (buff[i] != '\r'		    /* not eol */
		    && buff[i] != '\n'
		    && uedev.nlines == 0	    /* no line yet */
		    && uedev.isz == UEBUFSZ-1)  /* only 1 buf char left */
		    continue;		    /* ignore it */

		uedev.ibuff[obx] = buff[i];     /* store char in ibuff */

		obx = (obx + 1) % UEBUFSZ;	    /* increment buffer index */
		uedev.isz++;		    /* increment # of chars */

		if (buff[i] == '\r') {	    /* treat '\r' as eol */
		    uedev.nlines++;
		}
		if (buff[i] == '\n') {	    /* eol? */
		    uedev.nlines++;
		}
	    }
	}

	rc = uedev.ibuff[uedev.ibx];
	uedev.ibx = (uedev.ibx + 1) % UEBUFSZ;
	uedev.isz--;
	if (rc == '\r' || rc == '\n') {
	    uedev.nlines--;
	    rc = '\n';
	}

	return (devcall)rc;
}
