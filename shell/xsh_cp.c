/* xsh_cp.c - xsh_cp */

/*---------------------------------------------------------*/
/* Copy a file.                                            */
/*---------------------------------------------------------*/

#include <xinu.h>
#include <stdio.h>
#include <string.h>

/*------------------------------------------------------------------------
 * xsh_cp - shell command to copy a file to a destination
 *------------------------------------------------------------------------
 */
shellcmd xsh_cp(int nargs, char *args[])
{
	did32	inf, outf;		/* descriptors for in, out	*/
	char	buff[1024];		/* data buffer */
	int32	n;			/* result of read/write */


	/* For argument '--help', emit help about the 'cat' command	*/

	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
		printf("Use: cp source destination\n\n");
		printf("Description:\n");
		printf("\tcopies the source file to the destination\n");
		printf("Options:\n");
		printf("\t--help\t display this help and exit\n");
		return 0;
	}

	if (nargs != 3) {
	    printf("Usage:\n");
	    printf("    cp --help    -or-\n");
	    printf("    cp source destination\n");
	    return 0;
	}


	inf = open(NAMESPACE,args[1],"ro");
	if (inf == SYSERR) {
	    printf("Cannot open %s for input.\n");
	    return 0;
	}
	remove(args[2],0);
	outf = open(NAMESPACE,args[2],"wn");
	if (outf == SYSERR) {
	    printf("Cannot open %s for output.\n");
	    close(inf);
	    return 0;
	}

	for(;;) {
	    n = read(inf,buff,1024);
	    if (n < 1)
		break;
	    write(outf,buff,n);
	}

	close(inf);
	close(outf);
	return 0;
}
