/* strlen.c - strlen */

/*------------------------------------------------------------------------
 * strlen - Compute the length of a null-terminated character string, not
 *			counting the null byte.
 *------------------------------------------------------------------------
 */
int strlen(const char *s)
{
    const char *p = s;

    while (*p++)
	;
    return p - s - 1;
}
