/* xsh_reboot.c - xsh_reboot */

#include <xinu.h>

/*------------------------------------------------------------------------
 * xsh_reboot  -  reboot the system
 *------------------------------------------------------------------------
 */

shellcmd xsh_reboot(int nargs, char *args[])
{
	if (nargs > 1) {
	    printf("This command will reboot the system.");
	    printf(" It has no arguments.\n");
	    return OK;
	}
	reboot();
	printf("Reboot failed!\n");
	return OK;
}
