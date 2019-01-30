/* Simple implemention of tail. Always prints 'n' lines. */

#include <xinu.h>
#include <stdlib.h>

#define BUFSIZE 512

static char buffer[BUFSIZE];		/* buffer to hold part of a file */
static uint32 filesize;			/* size of file, in bytes */

/*------------------------------------------------------------*/
/* Copy all bytes from position 'pos' through the end of file */
/* for device 'fd' to the standard output, then quit.         */
/*------------------------------------------------------------*/
static void docopy(did32 fd, int pos)
{
    int n;

    seek(fd,filesize-pos);
    for(;;) {
        n = read(fd,buffer,BUFSIZE);
        if (n > 0)
            write(stdout,buffer,n);
        if (n < BUFSIZE)
            break;
    }
    close(fd);
    exit();
}

shellcmd xsh_tail(int argc, char *argv[])
{
    did32 fd;			/* file device */
    int n;			/* # of lines to display */
    int nrd;
    int nlf, neol;
    int pos, newpos;
    int nread;
    int i, k;
    int d;

    if (argc == 2 && strcmp(argv[1],"--help") == 0) {
	printf("Usage: tail [-N] filename\n");
	printf("Display the last N (or fewer) lines of a file.\n");
	printf("The default value of N is 10.\n");
	return 0;
    }

    if (argc > 3) {
        printf("Usage: tail [-N] filename\n");
        return 0;
    }

    if (argc == 3) {
	if (argv[1][0] == '-') {
	    n = atoi(argv[1]+1);
	    if (n < 1) {
		printf("# of lines must be at least 1.\n");
		return 0;
	    }
	} else {
	    printf("Usage: tail [-N] filename\n");
	    return 0;
	}
    } else
	n = 10;

    fd = open(NAMESPACE, argv[argc-1], "ro");
    if (fd == (did32)SYSERR) {
	printf("Cannot open %s\n", argv[argc-1]);
	return 0;
    }

    filesize = control(fd,LF_CTL_SIZE,0,0);
    /* printf("File size = %d bytes.\n", filesize); */

    d = BUFSIZE;
    if (d > filesize)
        d = filesize;
    nrd = d;
    nlf = 0;
    pos = d;

    for(;;) {
        seek(fd,filesize-pos);		/* move to pos bytes before eof */
        nread = read(fd,buffer,nrd);    /* read nrd bytes */
        if (nread != nrd) {
            printf("Read error.\n");
            return 0;
        }
        neol = 0;
        for (i=0;i<nrd;i++)
            if (buffer[i] == '\n')
                neol++;
        if (neol + nlf > n) {
            k = neol + nlf - n;
            i = 0;
            while (k > 0) {
                if (buffer[i] == '\n')
                    k--;
                i++;
            }
            if (i < nrd)
                write(stdout,buffer+i,nrd-i);
            docopy(fd,pos-nrd);
        }
        nlf += neol;
        if (pos == filesize)
            docopy(fd,pos);
        newpos = pos + nrd;
        if (newpos > filesize) {
            newpos = filesize;
            nrd = newpos - pos;
        }
        pos = newpos;
    }
    return 0;                           /* not reached */
}
