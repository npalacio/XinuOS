/* putchar.c - putchar */

#include <xinu.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  putchar  -  Write a character to the standard output device.
 *------------------------------------------------------------------------
 */
int		putchar(
		  int		c
		)
{
    return fputc(c, stdout);
}
