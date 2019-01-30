/* xsh_sum.c - xsh_sum */

#include <xinu.h>
#include <stdio.h>
#include <string.h>

/*------------------------------------------------------------------------
 * xsh_sum  -  Shell command to display the sum of its arguments
 *------------------------------------------------------------------------
 */
shellcmd xsh_sum(int nargs, char *args[])
{
	int32	i;			/* argument counter		*/
	int32	gotone;			/* true if we got a digit	*/
	int32	sign;			/* sign of an argument		*/
	long	value;			/* value of an argument		*/
	int32	sum;			/* sum of the arguments		*/
	char	*chptr;			/* Walks through argument	*/
	char	ch;			/* Next character of argument	*/
	int32	s;			/* sum after adding an item	*/

	/* For argument '--help', emit help about the 'sleep' command	*/

	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
		printf("Use: %s\n\n", args[0]);
		printf("Description:\n");
		printf("\tDisplay the sum of the integer arguments.\n");
		printf("Options:\n");
		printf("\t--help\t display this help and exit\n");
		return 0;
	}

	/* Check for valid number of arguments */

	if (nargs < 2) {
		fprintf(stderr, "%s: too few arguments\n", args[0]);
		fprintf(stderr, "Try '%s --help' for more information\n",
				args[0]);
		return 1;
	}

	sum = 0;
	for (i=1; i<nargs; i++) {
	    gotone = 0;
	    value = 0;
	    chptr = args[i];

	    /* Check for '+' or '-' prefix. */

	    sign = 1;				/* assume positive argument */
	    if (*chptr == '-') {
		sign = -1;
		chptr++;			/* skip - sign */
	    } else if (*chptr == '+')
		chptr++;			/* skip + sign */

	    ch = *chptr++;			/* get first digit */
	    while (ch != NULLCH) {
		if ( (ch < '0') || (ch > '9') ) {
		    fprintf(stderr, "%s: nondigit in argument %s\n",
			    args[0], args[i]);
		    return 1;
		}
/* XXX We need to check the next calculation for overflow, too. */
		value = 10 * value + (ch - '0');
		gotone = 1;
		ch = *chptr++;
	    }

	    /* Verify we got at least one digit. */

	    if (gotone == 0) {
		fprintf(stderr, "%s: argument %s has no digits\n",
			args[0], args[i]);
		return 1;
	    }

	    value = sign * value;

	    /*--------------------------------------------------------*/
	    /* Check for overflow. Overflow on signed addition occurs */
	    /* if both operands have the same sign, and the sum has   */
	    /* a different sign.                                      */
	    /*--------------------------------------------------------*/
	    s = sum + value;
	    if ( (sum > 0 && value > 0 && s < 0) ||
	         (sum < 0 && value < 0 && s > 0) ) {
		fprintf(stderr,"Error: overflow after adding %s\n", args[i]);
		return 0;
	    }
	    sum = s;
	}
	printf("%d\n", sum);
	return 0;
}
