/* strtol.c - strtol */

/*------------------------------------------------------------------------
 *  strtol  -  Convert string to long, with checking
 *------------------------------------------------------------------------
 */

#include <xinu.h>
#include <ctype.h>
#include <limits.h>

long strtol(const char *s, char **endptr, int base)
{
    const char *sc;
    unsigned long x;
    for (sc=s; isspace(*sc); ++sc)
	;
    x = _Stoul(s, endptr, base);
    if (*sc == '-' && x <= LONG_MAX) {		/* neg number overflow */
	proctab[currpid].errno = ERANGE;
	return (LONG_MIN);
    } else if (*sc != '-' && LONG_MAX < x) {	/* pos number overflow */
	proctab[currpid].errno = ERANGE;
	return (LONG_MIN);
    } else
	return (long)x;
}
