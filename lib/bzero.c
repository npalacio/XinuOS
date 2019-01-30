/* bzero.c - bzero */

/*------------------------------------------------------------------------
 *  bzero  -  Sets each byte in a block to zero.
 *------------------------------------------------------------------------
 */
void	bzero(
		  void		*p,
		  int		len
		)
{
    int n;
    char *pch = (char *)p;

    if ((n = len) <= 0)
    {
        return;
    }
    do
    {
        *pch++ = 0;
    }
    while (--n);
}
