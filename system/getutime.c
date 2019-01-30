/* getutime.c - getutime */

#include <xinu.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * getutime  -  Obtain time in seconds past Jan 1, 1970, UTC (GMT)
 *------------------------------------------------------------------------
 * This version of getutime has been modified to use multiple time servers.
 * In the event one fails, it tries again with the next server. Only if all
 * of the servers fail to respond does this getutime respond with SYSERR.
 *------------------------------------------------------------------------
 */
status	getutime(
	  uint32  *timvar		/* Location to store the result	*/
	)
{
	uint32	nnow;			/* Current time in network fmt	*/
	uint32	now;			/* Current time in xinu format	*/
	int32	retval;			/* Return value from call	*/
	uid32	slot;			/* Slot in UDP table		*/
	uint32	serverip;		/* IP address of a time server	*/
	char	prompt[2] = "xx";	/* Message to prompt time server*/
	int32	tsnum;			/* which timeserver to use	*/
	int32	err;			/* non-zero if we've failed.	*/

	if (Date.dt_bootvalid) {	/* Return time from local info	*/
		*timvar = Date.dt_boot + clktime;
		return OK;
	}

	/*----------------------------------------------------------*/
	/* Get our local IP address. Guarantee tsaddr array is set. */
	/*----------------------------------------------------------*/
	if (getlocalip() == SYSERR) {
		fprintf(stderr,"getutime: getlocalip failed.\n");
		/* udp_release(slot);  -- slot undefined here */
		return SYSERR;
	}

	err = 0;
	for(tsnum=0;tsnum<NTSADDR;tsnum++) {		/* try all servers */

	    /*----------------------------------*/
	    /* Get the timeserver's IP address. */
	    /*----------------------------------*/
	    serverip = NetData.tsaddr[tsnum];
	    if (serverip == 0)				/* if no server here */
		continue;

	    /*-----------------*/
	    /* Get a UDP slot. */
	    /*-----------------*/
	    slot = udp_register(serverip, TIMERPORT, TIMELPORT);
	    if (slot == SYSERR) {
		/*---------------------------------------------------*/
		/* This failure isn't a function of which timeserver */
		/* we used. We'll fail on its first occurrence.      */
		/*---------------------------------------------------*/
		err = 1;
		break;
	    }

	    /*--------------------------------------------------*/
	    /* Send an arbitrary message to prompt time server. */
	    /*--------------------------------------------------*/
	    retval = udp_send(slot, prompt, 2);
	    if (retval == SYSERR) {
		udp_release(slot);
		err = 2;
		continue;	/* yes, not timeserver dependent? */
	    }

	    /*-------------------------------------*/
	    /* Wait for the timeserver's response. */
	    /*-------------------------------------*/
	    retval = udp_recv(slot, (char *) &nnow, 4, TIMETIMEOUT);
	    if (retval == SYSERR) {
		udp_release(slot);
		err = 3;	/* possibly timeserver related? */
		continue;
	    }
	    if (retval == TIMEOUT) {
		udp_release(slot);
		err = 4;	/* definitely server related */
		continue;
	    }

	    /*-----------------------------*/
	    /* Success! Return the result. */
	    /*-----------------------------*/
	    udp_release(slot);
	    now = ntim2xtim( ntohl(nnow) );
	    Date.dt_boot = now - clktime;
	    Date.dt_bootvalid = TRUE;
	    *timvar = now;
	    return OK;
	}

	/*---------------------------------------------*/
	/* Failed with all servers. Report last error. */
	/*---------------------------------------------*/
	switch(err) {
	    case 1:
		fprintf(stderr,"getutime: cannot register a udp port %d\n",
					TIMERPORT);
		break;
	    case 2:
		fprintf(stderr,"getutime: cannot send a udp message %d\n",
					TIMERPORT);
		break;
	    case 3:
		fprintf(stderr,"getutime: udp_recv failed.\n");
		break;
	    case 4:
		fprintf(stderr,"getutime: udp_recv timed out.\n");
		break;
	    default:
		fprintf(stderr,"getutime: bad error code.\n");
	}
	return SYSERR;
}
