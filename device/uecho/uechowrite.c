/* uechowrite.c - uechowrite */

#include <xinu.h>

extern struct uedevice uedev;

/*------------------------------------------------------------------------
 *  uechowrite  -  Write character(s) to a uecho device
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

devcall	uechowrite(
	  struct dentry	*devptr,	/* Entry in device switch table	*/
	  char	*buff,			/* Buffer of characters		*/
	  int32	count 			/* Count of characters to write	*/
	)
{
	int32 nmove;			/* # of bytes to copy to buffer */

	if (!uedev.ueisopen) 		/* device not open? return SYSERR */
	    return (status)SYSERR;

	if (count < 0)			/* can't write less than 0 chars */
	    return SYSERR;

	wait(uedev.osem);		/* wait for output buffer access */

	while (count > 0) {
	    nmove = UEOBUFSZ - uedev.obx;	/* how much to copy */
	    if (nmove > count)
		nmove = count;

	    memcpy(uedev.obuff+uedev.obx,	/* copy: destination */
		buff, nmove);			/* source, count */
	    buff += nmove;		/* adjust incoming buffer address */
	    count -= nmove;		/* adjust count left to send */
	    uedev.obx += nmove;		/* adjust output buffer index */

	    if (uedev.obx == UEOBUFSZ) {	/* if the buffer is full */
		udp_sendto(uedev.slot,	/* send the data */
		    uedev.remip,
		    uedev.remport,
		    uedev.obuff,
		    UEOBUFSZ);
		uedev.obx = 0;		/* reset the output index */
	    }
	}
	signal(uedev.osem);
	return (status)OK;
}
