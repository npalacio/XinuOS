#include <xinu.h>

/* toupper.c - toupper */

/*------------------------------------------------------------------------
 * toupper - If the argument is a lower-case alphabetic character, convert
 *	     it to upper case. Otherwise return it unchanged.
 *------------------------------------------------------------------------
 */
int32	toupper(
	  int32	c				/* character to convert */
	)
{
	if (c >= 'a' && c <= 'z')		/* is c lowercase? */
	    return c - 'a' + 'A';		/* if so, convert and return */

	return c;				/* return unchanged value */
}
