/* uechocontrol.c - uechocontrol */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  uechocontrol  -  Control a uecho device
 *------------------------------------------------------------------------
 */

extern struct uedevice uedev;

devcall	uechocontrol(
	  struct dentry	*devptr,	/* Entry in device switch table	*/
	  int32	 func,			/* Function to perform		*/
	  int32	 arg1,			/* Argument 1 for request	*/
	  int32	 arg2			/* Argument 2 for request	*/
	)
{
	return (devcall)OK;
}
