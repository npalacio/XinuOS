#include <xinu.h>

/* tolower.c - tolower */

/*------------------------------------------------------------------------
 * tolower - If the argument is an upper-case alphabetic character, convert
 *	     it to lower case. Otherwise return it unchanged.
 *------------------------------------------------------------------------
 */
int32	tolower(
	  int32 c				/* character to convert */
	)
{
	if (c >= 'A' && c <= 'Z')		/* is c uppercase? */
	    return c - 'A' + 'a';		/* if so, convert and return */

	return c;				/* return unchanged value */
}
