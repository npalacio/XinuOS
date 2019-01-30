/* uechoinit.c - uechoinit, ueproc, ueconn */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  uechoinit  -  Initialize uecho device
 *------------------------------------------------------------------------
 */
struct uedevice uedev;

devcall	uechoinit(
	  struct dentry	*devptr		/* Entry in device switch table	*/
	)
{
	uedev.ueisopen = 0;		/* device is not open yet */
	uedev.reg = 0;			/* port is not yet registered */
	uedev.osem = semcreate(1);	/* output buffer semaphore */
	uedev.obx = 0;			/* output buffer pointer */

	return (devcall)OK;
}

/*------------------------------------------------------------------------
 * ueproc  -  Send UE0 output buffer, as appropriate
 *------------------------------------------------------------------------
 */

process	ueproc ()
{
	/*----------------------------------------------*/
	/* Do forever					*/
	/*	sleep however long is reasonable	*/
	/*	down uesema				*/
	/*	if ue0 output buffer is not empty	*/
	/*		send it				*/
	/*		empty the buffer		*/
	/*	up uesema				*/
	/*----------------------------------------------*/

	while(1) {
		sleepms(UEOWAIT);		/* wait a while... */
		wait(uedev.osem);
		if (uedev.obx > 0) {		/* output buffer not empty? */
		    udp_sendto(uedev.slot,	/* send the data */
			uedev.remip,
			uedev.remport,
			uedev.obuff,
			uedev.obx);
		    uedev.obx = 0;		/* reset the output index */
		}
		signal(uedev.osem);
	}
}

/*------------------------------------------------------------------------
 * ueconn  -  wait for a connection request, start a shell, and repeat
 *------------------------------------------------------------------------
 */

process	ueconn ()
{
    did32 descr;		/* XXX - perhaps put this in uedev struct? */
    pid32 ueconnpid;		/* XXX - likewise */
    int32 msg;			/* message from receive when shell ends. */

    for(;;) {
	/*-------------------------------------------------------------*/
	/* Open the UE0 device. This will block until the utel program */
	/* is executed on the remote system and contact this Xinu box. */
	/*-------------------------------------------------------------*/
	descr = open(UE0,"unused","unused");

	/*--------------------------------------------------------------*/
	/* Now start a shell that communicates using the descriptor for */
	/* the UE0 device. Panic if the process creation fails.         */
	/*--------------------------------------------------------------*/
	ueconnpid = create(shell, 8192, 50, "sshell", 1, descr);
	if (ueconnpid == SYSERR) {
	    panic("Cannot create uecho connection process");
	}

	/*-----------------------------------------*/
	/* Resume the shell process, and then wait */
	/* for it to terminate.                    */
	/*-----------------------------------------*/
	msg = recvclr();
	resume(ueconnpid);
	msg = receive();
	while (msg != ueconnpid)
	    msg = receive();
    }
}
