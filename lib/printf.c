#if 0
/* XXX OLD VERSION */
/* printf.c - printf*/

#include <xinu.h>
#include <stdio.h>
#include <stdarg.h>

extern void _fdoprnt(char *, va_list, int (*)(did32, char), int);

/*------------------------------------------------------------------------
 *  printf  -  print a formatted message on the standard output device.
 *------------------------------------------------------------------------
 */
int		printf(
		  const char		*fmt,
		  ...
		)
{
    va_list ap;
    syscall putc(did32, char);

    va_start(ap, fmt);
    _fdoprnt((char *)fmt, ap, putc, stdout);
    va_end(ap);

    return 0;
}
#endif


#include <xinu.h>
#include <stdarg.h>

extern void _fdoprnt(const char *, va_list, int (*func) (int, int), int);
static int sprntf(int, int);

/*------------------------------------------------------------------------
 *  printf  -  print a formatted message on the standard output device.
 *		Return 0 if the output was printed successfully,
 *		-1 if an error occurred.
 *------------------------------------------------------------------------
 */
int		printf(
		  const char	*fmt,	/* format string */
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
    write(stdout,buff,strlen(buff));

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
