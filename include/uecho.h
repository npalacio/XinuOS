/* uecho.h - constants for the uecho device type */

/************************************************************************/
/*									*/
/* 	Constants and declarations for the uecho device type		*/
/*									*/
/************************************************************************/

#define	UELPORT	42346		/* local UDP port for uecho	*/
#define UE0TIMEOUT 1000		/* pkt get timeout, in msec */
#define UEOWAIT 100		/* time between output proc iterations */

#define UEBUFSZ 2048		/* input buffer size */
#define UEOBUFSZ 512		/* output buffer size */

#define UESTK 2048		/* stack size for uecho process */
#define UEPRIO 500		/* priority for uecho process */

struct uedevice {
	int	ueisopen;	/* 0 if not open, non-zero otherwise */
	int	reg;		/* 0 if local port not registered */
	int	slot;		/* UDP table slot number */
	uint32	remip;		/* IP address of client */
	uint16	remport;	/* port number of client */
	uint16	cldisc;		/* non-zero if client has disconnected */
	sid32	osem;		/* output buffer semaphore */

	char	ibuff[UEBUFSZ];	/* incoming data buffer */
	uint16	ibx, isz;	/* index of first unused char, # of chars */
	uint16	nlines;		/* # of end-of-line chars in ibuff */

	char	obuff[UEOBUFSZ];	/* outgoing data buffer */
	uint16	obx;		/* index of next free output char */
};
