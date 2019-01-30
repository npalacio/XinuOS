/* xsh_sleep.c - xsh_sleep */

#include <xinu.h>

/*------------------------------------------------------------------------
 * xsh_sleep  -  Shell command to delay for a specified number of seconds
 *------------------------------------------------------------------------
 */
shellcmd xsh_sleep(int nargs, char *args[])
{
	int32	delay;			/* Delay in seconds		*/
	char	mopt;			/* was -m option present?	*/
	char	*chptr;			/* Walks through argument	*/
	char	ch;			/* Next character of argument	*/

	/* For argument '--help', display usage information. */

	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
	    printf("Usage:\n");
	    printf("    sleep --help		display this help\n");
	    printf("    sleep N			delay for N seconds\n");
	    printf("    sleep -m N		delay for N milliseconds\n");
	    return 0;
	}

	/*----------------------*/
	/* Process any options. */
	/*----------------------*/
	mopt = 0;
	while (nargs > 1 && args[1][0] == '-') {
	    if (!strcmp(args[1],"-m")) {	/* do milliseconds? */
		mopt = 1;
		nargs--;
		args++;
		continue;
	    }
	    fprintf(stderr,"Unknown option (%s)\n", args[1]);
	    return 0;
	}

	/* Check for valid number of arguments */

	if (nargs > 2) {
		fprintf(stderr, "%s: too many arguments\n", args[0]);
		fprintf(stderr, "Try '%s --help' for more information\n",
				args[0]);
		return 1;
	}

	if (nargs != 2) {
		fprintf(stderr, "%s: argument in error\n", args[0]);
		fprintf(stderr, "Try '%s --help' for more information\n",
				args[0]);
		return 1;
	}

	chptr = args[1];
	ch = *chptr++;
	delay = 0;
	while (ch != NULLCH) {
		if ( (ch < '0') || (ch > '9') ) {
			fprintf(stderr, "%s: nondigit in argument\n",
				args[0]);
			return 1;
		}
		delay = 10*delay + (ch - '0');
		ch = *chptr++;
	}
	if (mopt)
	    sleepms(delay);
	else
	    sleep(delay);

	return 0;
}
