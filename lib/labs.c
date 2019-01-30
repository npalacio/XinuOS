/* labs.c - labs */

/*------------------------------------------------------------------------
 *  labs  -  Returns the absolute value of a long integer.
 *------------------------------------------------------------------------
 */
long		labs(
			  long		arg
			)
{
    if (arg < 0)
        arg = -arg;
    return (arg);
}
