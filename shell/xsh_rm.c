/* xsh_rm.c - xsh_rm */

/*---------------------------------------------------------*/
/* Remove a file.                                          */
/*---------------------------------------------------------*/

#include <xinu.h>
#include <stdio.h>
#include <string.h>

/*------------------------------------------------------------------------
 * xsh_rm - shell command to remove a file
 *------------------------------------------------------------------------
 */
shellcmd xsh_rm(int nargs, char *args[])
{
	/* For argument '--help', emit help about the 'cat' command	*/

	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
		printf("Use: rm name ...\n\n");
		printf("Description:\n");
		printf("\tremoves the named files\n");
		printf("Options:\n");
		printf("\t--help\t display this help and exit\n");
		return 0;
	}

	while (nargs > 1) {
	    if (remove(args[1],0) != OK) {
		printf("%s not removed\n", args[1]);
		printf("errno = %d\n", proctab[currpid].errno);
	    }
	    nargs--;
	    args++;
	}
	return 0;
}
