/* regevent.c - regevent */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  regevent  -  register/unregister event handler for a specified event
 *------------------------------------------------------------------------
 */
syscall	regevent(
	  void *eh,			/* event handler func pointer	*/
	  uint32 event			/* event ID			*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/

	mask = disable();

	/* XXX Additional Work Required XXX */

	restore(mask);		/* Restore interrupts */
	return OK;
}
