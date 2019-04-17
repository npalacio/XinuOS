/* event.h - definitions for event handling functions */

/* Operating system events */
#define	PROCESS_GETMEM_EVENT	0	/* Sent by successfull getmem call */
#define	PROCESS_FREEMEM_EVENT	1	/* Send by successfull freemem call */

/* Limits on event numbers */
#define	MIN_USER_EVENT		8	/* Lowest event # user proc can send */
#define MAX_EVENTS		16	/* Max # events system can support */
