/* betup.c - netup */

#include <xinu.h>

extern uint32 network_status;		/* 0 = net down, 1 = net up	*/

/*------------------------------------------------------------------------
 *  netup - get or set network status
 *------------------------------------------------------------------------
 */
syscall	netup(
	  int32	k			/* 0, 1 = set status, 2 = query	*/
	)
{
	switch(k) {
	    case 0:
		network_status = 0;
		return 0;
	    case 1:
		network_status = 1;
		return 1;
	    case 2:
		return (int32)network_status;
	    default:
		return SYSERR;
	}
}
