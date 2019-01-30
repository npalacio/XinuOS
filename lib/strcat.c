/* strcat.c - strcat */

/*------------------------------------------------------------------------
 *  strcat  -  Concatenate s2 on the end of s1.
 *------------------------------------------------------------------------
 */
char *strcat(
	  char *s1,		/* first string */
	  const char *s2	/* second string */
	)
{
    char *s;

    s = s1;
    while (*s1++)
        ;
    --s1;			/* find end of s1 */
    while ((*s1++ = *s2++))
	;			/* copy s2 */
    return s;
}
