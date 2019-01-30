/* fprintf.c - _fdoprnt */

/*--------------------------------------------------------------------*/
/* This version essentially is sprintf to a (relatively) large buffer */
/* followed by a 'write' of the result using a single system call.    */
/*--------------------------------------------------------------------*/

#include <xinu.h>
#include <stdarg.h>

extern void _fdoprnt(char *, va_list, int (*func) (int, int), int);
static int sprntf(int, int);

/*------------------------------------------------------------------------
 *  fprintf  -  Print a formatted message on specified device (file).
 *				Return 0 if the output was printed successfully,
 *				-1 if an error occurred.
 *------------------------------------------------------------------------
 */
int		fprintf(
		  int		dev,			/* device to write to				*/
		  char		*fmt,			/* format string					*/
		  ...
		)
{
    va_list ap;
    char *s;
    char buff[512];			/* big enough? */

    s = buff;
    va_start(ap, fmt);
    _fdoprnt(fmt, ap, sprntf, (int)&s);
    va_end(ap);
    *s++ = '\0';
    write(dev,buff,strlen(buff));

    return 0;
}

/*------------------------------------------------------------------------
 *  sprntf  -  Routine called by _doprnt to handle each character.
 *------------------------------------------------------------------------
 */
static int		sprntf(
				  int		acpp,
				  int		ac
				)
{
    char **cpp = (char **)acpp;
    char c = (char)ac;

    return (*(*cpp)++ = c);
}
