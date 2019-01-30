/* strcpy.c - strcpy */

/*------------------------------------------------------------------------
 * strcpy - Copy the string given by the second argument into the first.
 *------------------------------------------------------------------------
 */
char *strcpy(
	  char	*tar,		/* target string	*/
	  const char *src	/* source string	*/
	)
{
	char *s = tar;

	while ( (*tar++ = *src++) != '\0')
		;
	return s;
}
