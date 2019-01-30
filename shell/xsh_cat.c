/* xsh_cat.c - xsh_cat */

/*--------------------------------------------------------------------*/
/* This is a modified version of cat. It correctly identifies the end */
/* of file.                                                           */
/*--------------------------------------------------------------------*/

#include <xinu.h>
#include <stdio.h>
#include <string.h>

/*------------------------------------------------------------------------
 * xsh_cat - shell command to cat one or more files
 *------------------------------------------------------------------------
 */
shellcmd xsh_cat(int nargs, char *args[])
{
	int32	i;			/* index into proctabl		*/
	did32	descr;			/* descriptor for a file	*/
	char	*argptr;		/* pointer to next arg string	*/

	char	buff[1024];		/* data buffer */
	int32	n;			/* result of read/write */


	/* For argument '--help', emit help about the 'cat' command	*/

	if (nargs == 2 && strcmp(args[1], "--help") == 0) {
		printf("Use: %s [file...]\n\n", args[0]);
		printf("Description:\n");
		printf("\twrites contents of files or stdin to stdout\n");
		printf("Options:\n");
		printf("\tfile...\tzero or more file names\n");
		printf("\t--help\t display this help and exit\n");
		return 0;
	}

	if (nargs == 1) {
		for(;;) {
		    n = read(stdin,buff,1024);
		    if (n < 1)
			break;
		    write(stdout,buff,n);
		}
		return 0;
	}
	for (i = 1; i < nargs; i++) {
		argptr = args[i];
		if ( (argptr[0] == '-') && (argptr[1] == NULLCH) ) {
			descr = stdin;
		} else {
			descr = open(NAMESPACE, argptr, "ro");
			if (descr == (did32)SYSERR) {
				fprintf(stderr, "%s: cannot open file %s\n",
					args[0], argptr);
#if 1
				fprintf(stderr, "errno = %d\n",
				    proctab[currpid].errno);
#endif
				return 1;
			}
		}
		for(;;) {
		    n = read(descr,buff,1024);
		    if (n < 1)
			break;
		    write(stdout,buff,n);
		}
		close(descr);
	}
	return 0;
}
