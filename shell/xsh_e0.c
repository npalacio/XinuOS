
/*-------------------------------------------------------------------------
 * e0 - A Very Simple Text Editor for XINU
 *-------------------------------------------------------------------------
 */

#include <xinu.h>

#define LOCAL static

/*=========================================================================
 * An "extended" malloc for XINU
 *
 * The freemem system call in Xinu requires that the size of the region
 * being deallocated be specified. This is an encumberance on the process,
 * since the size must then be maintained for each allocation. This pair of
 * functions, xmalloc and xfree, keep track of the size of each allocation
 * in storage that immediately precedes the user portion of the allocation.
 * It also keeps track of all the allocations made by the process, enabling
 * all dynamically-allocated storage to be returned at the end of execution
 * of the process.
 *
 * The allocated memory looks like this:
 *
 *	+-------------------+
 *	| ptr to next block | <--- Address returned by getmem
 *	+-------------------+
 *	|  allocation size  |
 *	+-------------------+
 *	.		    . <--- Address returned to user
 *	. user's allocation .
 *	.		    .
 *	+-------------------+
 * 
 *=========================================================================
 */

/*------------------------------------------------------------------------
 * List of dynamically allocated blocks of memory for the process
 *------------------------------------------------------------------------
 */
struct xblock {
    struct xblock *next;		/* pointer to the next xblock */
    uint32 nbytes;			/* size of the allocation */
    /* The actual user allocation appears here. */
};
static struct xblock *pmlist = NULL;	/* list of all allocations */

char* xmalloc(uint32 nbytes)
{
    char *mp;
    struct xblock *xmp;

    if (nbytes == 0) {
	panic("xmalloc: 0 bytes requested\n");
        return NULL;			/* not reached */
    }
    nbytes += sizeof(struct xblock);
    mp = (char *)getmem(nbytes);
    if (mp == (char *)SYSERR) {
        panic("xmalloc: memory depleted\n");
        return (char *)SYSERR;		/* not reached */
    }
    xmp = (struct xblock *)mp;
    xmp->next = pmlist;
    xmp->nbytes = nbytes;
    pmlist = xmp;
    return (char *)xmp + sizeof(struct xblock);
}

/*--------------------------------------------------------------------------
 * xfree -- free memory allocated by xmalloc or xcalloc
 *--------------------------------------------------------------------------
 */
static void xfree(char *mp)
{
    struct xblock *xmp, *curr, *prev;

    if (mp == NULL)
	panic("xfree: NULL argument");
    xmp = (struct xblock *)((char *)mp - sizeof(struct xblock));

    /*-----------------------------------------*/
    /* Search the list to find the allocation. */
    /*-----------------------------------------*/
    prev = NULL;
    curr = pmlist;
    while (curr != NULL && curr != xmp) {
	prev = curr;
	curr = curr->next;
    }
    if (curr == NULL) {
	panic("xfree: trying to delete an unallocated block.");
	return;				/* not reached */
    }
    if (prev == NULL)			/* update head pointer */
	pmlist = curr->next;
    else
	prev->next = curr->next;
    freemem((char *)curr,curr->nbytes);	/* deallocate */
}

/*--------------------------------------------------------------------------
 * Free all remaining dynamic allocations
 *--------------------------------------------------------------------------
 */
static void freeall(void)
{
    struct xblock *xp, *nxp;

    xp = pmlist;
    while (xp != NULL) {
	nxp = xp->next;
	freemem((char *)xp,xp->nbytes);
	xp = nxp;
    }
}

/*----------------------------------------*/
/* Return non-zero if dev is the console. */
/* XXX Later we may have to deal with     */
/* XXX additional TTY devices.            */
/*----------------------------------------*/
#if 0
static int isatty(int dev)
{
    return dev == CONSOLE;
}
#endif

#define MAXLINE 80
#define P(s) fprintf(stdout,s)

struct tline {
    struct tline *prev;			/* ptr to previous tline */
    struct tline *next;			/* ptr to next tline */
    int len;				/* length of the line */
    char txt[MAXLINE+1];		/* text of the current line */
};
typedef struct tline tline_t;

LOCAL char line[MAXLINE+1];
LOCAL tline_t *txthd, *txttl;		/* ptrs to head, tail of text lines */
LOCAL tline_t *pline;			/* ptr to current line being edited */
LOCAL int plnum;			/* line number of pline */
LOCAL int nlines;			/* # of lines */
LOCAL int nleft;
LOCAL int visible;			/* 1 to show line after move */
LOCAL int dev;				/* device for current file */
LOCAL int pid;				/* e0 process ID */
LOCAL int new;				/* 1 if file was just created */
LOCAL char fname[80];			/* name of file being edited */
LOCAL char bname[80];			/* name with ".bak" extension */
LOCAL char cc;				/* command character */
LOCAL int numarg;			/* numeric command argument */
LOCAL char *strarg;			/* ptr to str arg (in 'line') */
LOCAL int istrunc;			/* 1 if getline truncated line */
LOCAL int ntrunc;			/* # of truncated lines in file */

/*--------------------------------------------------------------*/
/* Get the next line from 'dev' into 'line' (max len = MAXLINE) */
/* Don't include the terminating end of line character, but do  */
/* end with a null byte.                                        */
/* Return the number of bytes in the line, excluding the end of */
/* line character. Return EOF at end of file.                   */
/*--------------------------------------------------------------*/
static int getline(int dev)
{
    char *p = line;
    int n, len = 0;
    char c;

    istrunc = 0;
    for(;;) {
	n = read(dev,&c,1);
	if (n == SYSERR) {
	    printf("Error reading device %d in getline.\n", dev);
	    freeall();
	    kill(getpid());
	}
	if (n == EOF)
	    return EOF;
	if (n == 0) {
	    if(isatty(dev)) break;
	    return EOF;
	}
	if (c == '\n') break;
	if (len < MAXLINE-1) {
	    *p++ = c;
	    len++;
	} else istrunc = 1;
    }
    *p = '\0';
    return len;
}

/*-----------------------------------------*/
/* Display a failure message and terminate */
/*-----------------------------------------*/
void fail(int code)
{
    char *msg;

    switch(code) {
        case 1:    msg = "out of memory"; break;
        case 2:    msg = "usage: e0 filename"; break;
        case 3:    msg = "unable to open or create file"; break;
        case 4:    msg = "error closing file"; break;
        case 5:    msg = "error reading file"; break;
        case 6:    msg = "bad end of line in input file"; break;
        case 7:    msg = "line too long in input file"; break;
        case 8:    msg = "inconsistency in line structure (goline)"; break;
        case 9:    msg = "inconsistency in args to insert"; break;
        case 10:   msg = "unable to rename file to *.bak"; break;
        case 11:   msg = "unable to create file"; break;
        case 12:   msg = "error writing file"; break;
        case 13:   msg = "inconsistency in delline"; break;
	case 14:   msg = "unable to write new file; backup is valid"; break;
	case 15:   msg = "unable to reopen file (save)"; break;
	case 16:   msg = "txthd or txttl incorrect in insert"; break;
        default:   msg = "bad failure code"; break;
    }
    fprintf(stderr,"%s\n",msg);
    freeall();
    kill(getpid());
}

/*--------------------------------------------------*/
/* Display 'n' (or fewer) lines starting with pline */
/*--------------------------------------------------*/
void printn(int n)
{
    tline_t *pl;

    pl = pline;
    while (pl != (tline_t *)NULL && n > 0) {
	fprintf(stdout,"%s\n",pl->txt);
	pl = pl->next;
	n--;
    }
}

/*----------------------------------------------------------------*/
/* Read the text of the current file into the line data structure */
/*----------------------------------------------------------------*/
static void readfile(void)
{
    int len;
    tline_t *p;

    ntrunc = 0;
    len = getline(dev);
    while (len != EOF) {
	if (istrunc) ntrunc++;
	p = (tline_t *)xmalloc(sizeof(tline_t));
	if (p == NULL) fail(1);
	p->prev = txttl;
	p->next = NULL;
	p->len = len;
	strcpy(p->txt,line);
	if (txthd == NULL) txthd = p;
	else txttl->next = p;
	txttl = p;
	nlines++;
	len = getline(dev);
    }
    if (nlines == 0) {
	pline = (tline_t *)NULL;
	plnum = 0;
    } else {
	pline = txthd;
	plnum = 1;
    }
    if (ntrunc > 0) fprintf(stdout,"Warning: %d lines truncated.\n", ntrunc);
}

/*--------------------------*/
/* Display help information */
/*--------------------------*/
static void help(void)
{
    fprintf(stdout,"Commands\n");
    P("commands (# = decimal number)\n");
    P("-----------------------------\n");
    P("a\tinsert lines after the current line.\n");
    P("d[#]\tdelete # lines (default 1)\n");
    P("e\tsave file and exit\n");
    P("g[#]\tgo to line # (default 1)\n");
    P("h\tdisplay this help\n");
    P("i\tinsert lines before current line\n");
    P("p[#]\tprint # lines starting with current line (default 1)\n");
    P(".\tsame as p1\n");
    P("q\tquit without saving changes\n");
    P("v\tstop/start showing current line after line position changes\n");
    P("+[#]\tmove forward # lines (default 1)\n");
    P("blank line is equivalent to +1\n");
    P("-[#]\tmove backward # lines (default 1)\n");
    P("$\tmove to the last line\n");
    P("/s\tsearch forward for first line containing \"s\"\n");
    P("?s\tsearch backward for first line containing \"s\"\n");
    P("\n");
    P("End lines to insert or append with a line containing a single .\n");
}

/*---------------------------------------------*/
/* Check the health of the line data structure */
/*---------------------------------------------*/
static void validate(void)
{
    int nl;
    tline_t *pl, *opl;

    if (txthd == (tline_t *)NULL) {
	if  (txttl != (tline_t *)NULL) {
	    fprintf(stderr,"txthd is null, but txttl is not.\n");
	}
	if (pline != (tline_t *)NULL) {
	    fprintf(stderr,"txthd is null, but pline is not.\n");
	}
	if (plnum != 0) {
	    fprintf(stderr,"txthd is null, but plnum is not zero.\n");
	}
	if (nlines > 0) {
	    fprintf(stderr,"txthd is null, but nlines is not zero.\n");
	}
	return;
    }
    if (txttl == (tline_t *)NULL) {
	if  (txthd != (tline_t *)NULL) {
	    fprintf(stderr,"txttl is null, but txthd is not.\n");
	}
	if (pline != (tline_t *)NULL) {
	    fprintf(stderr,"txttl is null, but pline is not.\n");
	}
	if (plnum != 0) {
	    fprintf(stderr,"txttl is null, but plnum is not zero.\n");
	}
	if (nlines > 0) {
	    fprintf(stderr,"txttl is null, but nlines is not zero.\n");
	}
	return;
    }

    /* Check links, knowing that txthd and txttl are both non-NULL */
    if (txthd->prev != (tline_t *)NULL) {
	fprintf(stderr,"Line 1: bad prev pointer.\n");
    }
    if (txttl->next != (tline_t *)NULL) {
	fprintf(stderr,"Last line: bad next pointer.\n");
    }
    pl = txthd;
    nl = 0;
    while (pl != (tline_t *)NULL) {
	nl++;
	opl = pl;
	pl = pl->next;
	if (pl != (tline_t *)NULL && pl->prev != opl) {
	    fprintf(stderr,"Inconsistent links between lines %d and %d\n",
		nl, nl+1);
	    return;
	}
    }
    if (nl != nlines) {
	fprintf(stderr,"Line count wrong; %d actual lines, nlines = %d\n",
	    nl, nlines);
    }
}

/*--------------------------------------------------*/
/* Delete 'n' lines starting with the current line. */
/*--------------------------------------------------*/
static void delline(int n)
{
    int i;
    tline_t *prev, *next;

    for (i=0;i<n;i++) {
	if (txttl == (tline_t *)NULL) {
	    if (txttl != (tline_t *)NULL) fail(13);
	    pline = (tline_t *)NULL;
	    plnum = 0;
	    nlines = 0;
	    return;
	}
	prev = pline->prev;
	next = pline->next;
	xfree((char *)pline);
	if (prev == (tline_t *)NULL) txthd = next;
	else prev->next = next;
	if (next == (tline_t *)NULL) txttl = prev;
	else next->prev = prev;
	nlines--;
	if (next != (tline_t *)NULL) pline = next;	/* plnum: no chg */
	else if (prev != (tline_t *)NULL) {
	    pline = prev;
	    plnum--;
	} else {
	    pline = (tline_t *)NULL;
	    plnum = 0;
	}
    }
}

/*--------------------------------------------------------------*/
/* Insert the tline structure pointed to by 'new' between those */
/* tline structures pointed to by 'prev' and 'next'. 'prev' or  */
/* 'next' or both may be NULL to accommodate insertions at the  */
/* beginning or end of a possibly empty file. Update the number */
/* of lines in the file, but don't modify plnum.                */
/*--------------------------------------------------------------*/
static void tinsert(tline_t *new, tline_t *prev, tline_t *next)
{
    /*------------------------------------*/
    /* Inserting the first line in a file */
    /*------------------------------------*/
    if (txthd == (tline_t *)NULL) {
	if (prev != (tline_t *)NULL) fail(9);
	if (next != (tline_t *)NULL) fail(9);
	txthd = txttl = new;
	new->prev = new->next = (tline_t *)NULL;

    /*-----------------------------------------------------*/
    /* Insertion before the first line in a non-empty file */
    /*-----------------------------------------------------*/
    } else if (prev == (tline_t *)NULL) {	/* and next != NULL */
	if (next == (tline_t *)NULL) fail(9);
	if (txthd == (tline_t *)NULL) fail(16);
	if (txttl == (tline_t *)NULL) fail(16);
	if (txthd != next) fail(9);
	new->prev = (tline_t *)NULL;
	new->next = txthd;
	next->prev = new;
	txthd = new;

    /*---------------------------------------------------*/
    /* Insertion after the last line in a non-empty file */
    /*---------------------------------------------------*/
    } else if (next == (tline_t *)NULL) {	/* and prev != NULL */
	if (prev == (tline_t *)NULL) fail(9);
	if (txthd == (tline_t *)NULL) fail(16);
	if (txttl == (tline_t *)NULL) fail(16);
	if (txttl != prev) fail(9);
	new->next = (tline_t *)NULL;
	txttl = new;
	new->prev = prev;
	prev->next = new;

    /*---------------------------------------------*/
    /* Insertion between lines in a non-empty file */
    /*---------------------------------------------*/
    } else {					/* next and prev != NULL */
	if (prev == (tline_t *)NULL) fail(9);
	if (next == (tline_t *)NULL) fail(9);
	if (txthd == (tline_t *)NULL) fail(16);
	if (txttl == (tline_t *)NULL) fail(16);
	new->prev = prev;
	new->next = next;
	prev->next = new;
	next->prev = new;
    }
    nlines++;
}

/*----------------------------------------------------*/
/* Return 1 if 's' appears in p->txt, and 0 otherwise */
/*----------------------------------------------------*/
static int match(char *s, tline_t *p)
{
    int ls;				/* length of s */
    int i;

    if ((ls = strlen(s)) == 0) return 1;	/* null string matches all */
    if (ls > p->len) return 0;

    /* Ideally we'd use strstr here, but it's not in XINU's lib yet */
    for (i=0;i<=p->len-ls;i++) {
	if (!strncmp(s,p->txt+i,ls)) return 1;
    }
    return 0;
}

/*---------------------------------------------------------------*/
/* Search for first line containing s; dir = 1 for foward search */
/* Do *NOT* include current line in the search. If found, set    */
/* pline to the line where the match occurred. If not found, do  */
/* not change pline at all.                                      */
/*---------------------------------------------------------------*/
static void find(char *s, int dir)
{
    int xlnum;
    tline_t *xline;

    xlnum = plnum;
    xline = pline;
    while (xline != NULL) {
	if (dir) {
	    xline = xline->next;
	    xlnum++;
	} else {
	    xline = xline->prev;
	    xlnum--;
	}
	if (xline == NULL) {
	    fprintf(stdout,"--not found\n");
	    return;
	}
	if (match(s,xline)) {		/* check for match */
	    plnum = xlnum;
	    pline = xline;
	    if (visible) printn(1);
	    return;
	}
    }
    fprintf(stdout,"--not found\n");
}
    

/*-------------------------------*/
/* Adjust plnum after insertions */
/*-------------------------------*/
static void adjust(tline_t *lasti)
{
    tline_t *pl;

    if (lasti == (tline_t *)NULL) return;	/* no insertion */
    pl = txthd;
    plnum = 1;
    while (pl != pline) {
	plnum++;
	pl = pl->next;
    }
}

/*--------------------------------------*/
/* Insert lines before the current line */
/*--------------------------------------*/
static void insline(void)
{
    tline_t *lasti;			/* last inserted line */
    tline_t *prev, *next;		/* lines between which to insert */
    tline_t *p;				/* new line to be inserted */
    int len;

    lasti = (tline_t *)NULL;
    for(;;) {
	len = getline(stdin);			/* get next line */

	if (len == 1 && line[0] == '.') {	/* end of insertions? */
	    adjust(lasti);
	    return;
	}

	/* Allocate storage for the new line. If unavailable, */
	/* display a message, but allow editing to continue.  */
	p = (tline_t *)xmalloc(sizeof(tline_t));
	if (p == (tline_t *)NULL) {
	    fprintf(stdout,"--out of memory.\n");
	    return;
	}
	strcpy(p->txt,line);
	p->len = len;

	if (txthd == (tline_t *)NULL) {		/* first line in file */
	    prev = next = (tline_t *)NULL;
	} else if (lasti == (tline_t *)NULL) {	/* first insertion */
	    prev = pline->prev;
	    next = pline;
	} else {				/* not first insertion */
	    prev = lasti;
	    next = lasti->next;
	}
	tinsert(p,prev,next);
	lasti = p;
    }
}

/*-------------------------------------*/
/* Append lines after the current line */
/*-------------------------------------*/
static void appline(void)
{
    tline_t *lasti;			/* last inserted line */
    tline_t *prev, *next;		/* lines between which to insert */
    tline_t *p;				/* new line to be appended */
    int len;

    lasti = (tline_t *)NULL;
    for(;;) {
	len = getline(stdin);			/* get next line */

	if (len == 1 && line[0] == '.') {	/* end of insertions? */
	    adjust(lasti);
	    return;
	}

	/* Allocate storage for the new line. If unavailable, */
	/* display a message, but allow editing to continue.  */
	p = (tline_t *)xmalloc(sizeof(tline_t));
	if (p == (tline_t *)NULL) {
	    fprintf(stdout,"--out of memory.\n");
	    return;
	}
	strcpy(p->txt,line);
	p->len = len;
	if (txthd == (tline_t *)NULL) {		/* first line in file */
	    prev = next = (tline_t *)NULL;
	} else if (lasti == (tline_t *)NULL) {	/* first insertion */
	    prev = pline;
	    next = pline->next;
	} else {				/* not first insertion */
	    prev = lasti;
	    next = lasti->next;
	}
	tinsert(p,prev,next);
	lasti = p;
    }
}

/*---------------------------------------*/
/* Exit, abandoning any edits performed. */
/*---------------------------------------*/
static void quit(void)
{
    freeall();
    kill(pid);
}

/*-------------------------------------*/
/* Create string naming of backup file */
/*-------------------------------------*/
static void makebak(void)
{
    char *p;

    strcpy(bname,fname);
    p = bname + strlen(bname) - 1;
    while (p >= bname && *p != '.') p--;
    if (p >= bname) *p = '\0';
    strcat(bname,".bak");
}

/*-----------------------------------------*/
/* Save the current file and exit.	   */
/* If the file is new, just write it.	   */
/* Otherwise:				   */
/*    (1) delete any existing ".bak" file  */
/*    (2) rename current file to ".bak"	   */
/*    (3) create a new file		   */
/*    (4) write the new file		   */
/*-----------------------------------------*/
static void save(void)
{
    tline_t *pl;
    int status;
    int nwritten;

    if (!new) {
	makebak();			/* create ".bak" filename */
	remove(bname,0);		/* remove any old backup */
	proctab[currpid].errno = 0;
	if (rename(fname,bname) == SYSERR) {
	    fail(10);			/* rename current file as ".bak" */
	}
	dev = open(NAMESPACE,fname,"wn");	/* create a new file */
	if (dev == SYSERR)
	    fail(14);
	close(dev);
    }

    /*-----------------------------------------------*/
    /* Write the file contents to the output device. */
    /*-----------------------------------------------*/
    dev = open(NAMESPACE,fname,"ow");
    if (dev == SYSERR) fail(15);
    pl = txthd;
    nwritten = 0;
    while (pl != (tline_t *)NULL) {
	status = write(dev,pl->txt,pl->len);
	if (status == SYSERR) fail(12);
	status = write(dev,"\n",1);
	if (status == SYSERR) fail(12);
	pl = pl->next;
	nwritten++;
    }
    if (close(dev) == SYSERR) fail(4);
    fprintf(stdout,"%d lines written.\n",nwritten);
    freeall();
    kill(pid);
}

/*---------------------------------*/
/* Make line 'n' the current line. */
/*---------------------------------*/
static void goline(int n)
{
    if (n > nlines) n = nlines;
    if (n < 1) n = 1;
    if (nlines == 0) {
	pline = (tline_t *)NULL;
	plnum = 0;
	return;
    }
    pline = txthd;
    plnum = 1;
    while (plnum != n) {
	if (pline->next == (tline_t *)NULL) fail(8);
	pline = pline->next;
	plnum++;
    }
    if (visible) printn(1);
}

/*------------------------------------------------------*/
/* Move the current line forward (if 'forward' is true) */
/* or backward in the list by 'n' lines (or less).      */
/*------------------------------------------------------*/
static void ldelta(int forward, int n)
{
    int i;

    if (pline == (tline_t *)NULL) return;	/* no lines in file */
    for (i=0;i<n;i++) {
	if (forward) {
	    if (pline->next != (tline_t *)NULL) {
		pline = pline->next;
		plnum++;
	    } else {
		if (visible) printn(1);
		return;
	    }
	} else {
	    if (pline->prev != NULL) {
		pline = pline->prev;
		plnum--;
	    } else {
		if (visible) printn(1);
		return;
	    }
	}
    }
    if (visible) printn(1);
}

/*---------------------------------------------------------------------*/
/* Get the next command, which consists of a non-digit and an optional */
/* numeric argument, optionally separated by whitespace. If the first  */
/* character is numeric, assume a 'G' command was entered. We'll later */
/* handle the / and ? commands as special cases. The global numarg     */
/* is set to the numeric argument's value. If the numeric argument is  */
/* not present, then numarg defaults to 1.                             */
/* (What should we do about end of file?)                              */
/*---------------------------------------------------------------------*/
static char getcommand(void)
{
    int i, len;
    int more;

    numarg = 1;
    len = getline(stdin);
    i = 0;
    /* Skip leading whitespace; treat empty line as '+' command */
    for(;;) {
	if (i >= len) return '+';		/* no command present */
	if (line[i] != ' ' && line[i] != '\t') break;
	else i++;
    }
    if (line[i] >= 'A' && line[i] <= 'Z') cc = line[i++];
    else if (line[i] >= 'a' && line[i] <= 'z') cc = line[i++] - 0x20;
    else if (line[i] >= '0' && line[i] <= '9') cc = 'G';
    else cc = line[i++];

    if (cc == '/' || cc == '?') {		/* Find command? */
	strarg = &line[i];
	return cc;
    }

    /* Skip whitespace after command character */
    more = 1;
    while (more) {
	if (i >= len) return cc;		/* no numeric argument */
	more = line[i] == ' ' || line[i] == '\t';
	if (more) i++;
    }

    /* Check for a numeric argument */
    if (line[i] < '0' || line[i] > '9') {
	fprintf(stderr,"?\n");
	return ' ';				/* do nothing command */
    }

    /* Get numeric argument */
    numarg = line[i++] - '0';
    while (i < len && line[i] >= '0' && line[i] <= '9') {
	numarg = numarg * 10 + line[i++] - '0';
    }

    /* Guarantee remainder of line is whitespace */
    while (line[i] == ' ' || line[i] == '\t') i++;
    if (line[i] != '\0') {
	fprintf(stderr,"?\n");
	return ' ';				/* do nothing command */
    }

    return cc;
}

shellcmd xsh_e0(int nargs, char *args[])
{
    pid = getpid();
    if (nargs != 2) fail(2);

    if (strncmp(args[1],"--help",6) == 0 && args[1][6] == '\0') {
	P("e0 is a simple editor for Xinu.\n");
	help();
        kill(pid);
    }

    txthd = txttl = (tline_t *)NULL;
    nlines = 0;
    nleft = 0;
    visible = 1;

    /*-------------------------*/
    /* Open or create the file */
    /*-------------------------*/
    strcpy(fname,args[1]);		/* save the file's name */
    new = 0;
    dev = open(NAMESPACE,fname,"ro");	/* try to open an existing file */
    if (dev == SYSERR) {		/* doesn't exist... */
	dev = open(NAMESPACE,fname,"wn");	/* try to open a new file */
	if (dev == SYSERR) fail(3);
	new = 1;
	pline = (tline_t *)NULL;
	plnum = 0;
    } else readfile();
    if (close(dev) == SYSERR) fail(4);

    for(;;) {
	if (visible)
	    fprintf(stdout,"%d",plnum);
	fprintf(stdout,"> ");
	cc = getcommand();
	switch(cc) {
	    case 'h': case 'H': help(); break;
	    case 'd': case 'D': delline(numarg); break;
	    case 'i': case 'I': insline(); break;
	    case 'a': case 'A': appline(); break;
	    case '.':		printn(1); break;
	    case 'p': case 'P':	printn(numarg); break;
	    case 'q': case 'Q': quit(); /* doesn't return */
	    case 'e': case 'E': save(); /* doesn't return */
	    case 'g': case 'G': goline(numarg); break;
	    case '+':		ldelta(1,numarg); break;
	    case '-':		ldelta(0,numarg); break;
	    case 'v': case 'V':	visible = !visible; break;
	    case 'c': case 'C':	validate(); break;
	    case '$':		goline(nlines); break;
	    case '/':           find(strarg,1); break;
	    case '?':           find(strarg,0); break;
	    case ' ':		break;		/* do nothing */
	    default:
		fprintf(stdout,"Unrecognized command; use H for help.\n");
	}
    }
}
