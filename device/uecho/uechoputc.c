/* uechoputc.c - uechoput */

#include <xinu.h>

extern struct uedevice uedev;

/*------------------------------------------------------------------------
 *  uechoput  -  write a character to a uecho device
 *------------------------------------------------------------------------
 */

#if 0
    down(sema)
    While there is data left to be sent:
        Put the data (or as much as possible) in the buffer
        If the buffer is full
            udp_send it
            empty the buffer
            continue (from the top of the while loop)
        Else
            break
    up(sema)
#endif

status	uechoput(
	struct dentry *devptr,		/* entry in device switch table */
	char ch				/* character to write */
	)
{
	if (!uedev.ueisopen) {		/* device not open? return SYSERR */
	    return (status)SYSERR;
	}

	wait(uedev.osem);		/* wait for output buffer access */
	uedev.obuff[uedev.obx++] = ch;	/* add the character, bump index */
	if (uedev.obx == UEOBUFSZ) {	/* if the buffer is full */
	    udp_sendto(uedev.slot,	/* send the data */
		uedev.remip,
		uedev.remport,
		uedev.obuff,
		UEOBUFSZ);
	    uedev.obx = 0;		/* reset the buffer index */
	}
	signal(uedev.osem);
	return (status)OK;
}
