/* sendevent.c - sendevent */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  sendevent  -  handle invocation of a callback function for a user event
 *------------------------------------------------------------------------
 */
syscall	sendevent(
	  pid32		pid,		/* ID of process with callback	*/
	  uint32	event		/* ID of event to be reported	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/

	mask = disable();

	if (isbadpid(pid)) {		/* Verify the process ID */
		restore(mask);
		return SYSERR;
	}

	/* XXX Additional Work Required XXX */

	restore(mask);		/* Restore interrupts */
	return OK;
}
