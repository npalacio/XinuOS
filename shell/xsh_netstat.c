/* xsh_netstat.c - xsh_netstat */

#include <xinu.h>
#include <stdio.h>
#include <string.h>

/*------------------------------------------------------------------------
 * xsh_netstat  -  Shell command to display network on/off status, and
 *			possibly turn networking off
 *------------------------------------------------------------------------
 */
shellcmd xsh_netstat(int nargs, char *args[])
{
	uint32	netstat;		/* CUrrent network status	*/

	/* For argument '--help', emit help about the 'netstatus' command */

	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
		printf("Use: %s --off\n\n", args[0]);
		printf("Description:\n");
		printf("\tWith no options, display network on/off status\n");
		printf("Options:\n");
		printf("\t--help\t display this help and exit\n");
		printf("\t--off\t set the networking flag to off\n");
		return 0;
	}

	if (nargs == 2 && strncmp(args[1], "--off", 6) == 0) {
		switch(netup(0)) {
		    case 0:
			return 0;
		    case SYSERR:
			fprintf(stderr, "%s - error turning network off.\n",
			    args[0]);
			return 1;
		    default:
			fprintf(stderr, "%s - cannot turn network off.\n",
			    args[0]);
		}
		return 0;
	}

	/* Check for valid number of arguments */

	if (nargs > 1) {
		fprintf(stderr, "%s: too many arguments\n", args[0]);
		fprintf(stderr, "Try '%s --help' for more information\n",
				args[0]);
		return 1;
	}

	netstat = netup(2);		/* get current status */

	printf("Network status: ");
	if (netstat == 1)
	    printf("on\n");
	else if (netstat == 0)
	    printf("off\n");
	else
	    printf("unknown (error getting network status)\n");
	return 0;
}
