/* getchar.c - getchar */

#include <xinu.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  getchar  -  Read a character from the standard input device.
 *------------------------------------------------------------------------
 */
int		getchar(
		  void
		)
{
    return fgetc(stdin);
}
