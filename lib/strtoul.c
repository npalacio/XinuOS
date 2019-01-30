/* strtoul.c - strtoul */

/*------------------------------------------------------------------------
 *  strtoul  -  Convert string to unsigned long, with checking
 *------------------------------------------------------------------------
 * XXX - We probably need to do more work on this to make it consistent
 *       with the Linux/POSIX definition.
 */

#include <xinu.h>
#include <ctype.h>
#include <limits.h>

unsigned long strtoul(const char *s, char **endptr, int base)
{
    const char *sc;
    unsigned long x;
    for (sc=s; isspace(*sc); ++sc)
	;
    x = _Stoul(s, endptr, base);
    if (*sc == '-') {				/* negative not allowed */
	proctab[currpid].errno = ERANGE;
	return (ULONG_MAX);
    } else 
	return x;
}
