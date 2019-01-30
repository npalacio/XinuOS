/* xsh_memdump.c - xsh_memdump */

#include <xinu.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

static int err;				/* non-zero if parseval found error */
static	uint32	parseval(const char *);

/*-------------------------------------------------------------------*/
/* xsh_memdump - dump a region of memory by displaying values in hex */
/*			 and ascii                                   */
/*-------------------------------------------------------------------*/
shellcmd xsh_memdump(int nargs, char *args[])
{
    bool8 force = FALSE;	/* ignore address sanity checks	*/
    uint32 start;		/* starting address		*/
    uint32 length;		/* length of region to dump	*/
    uint32 l;			/* counts length during dump	*/
    int32 i;			/* counts words during dump	*/
    uint32 *addr;		/* address to dump		*/
    char *chptr;		/* character address to dump	*/
    char ch;			/* next character to print	*/

    /*--------------------------------------------------------------*/
    /* For argument '--help', display information about the command */
    /*--------------------------------------------------------------*/
    if (nargs == 2 && strcmp(args[1], "--help") == 0) {
	printf("Use: %s [-f] START LENGTH\n\n", args[0]);
	printf("Description:\n");
	printf("\tDumps LENGTH bytes of memory starting at\n");
	printf("\tthe specified START address (both START and\n");
	printf("\tLENGTH can be specified in decimal or hex)\n");
	printf("Options:\n");
	printf("\t-f\t\tignore sanity checks for addresses\n");
	printf("\tSTART\t\tmemory address at which to start\n");
	printf("\tLENGTH\tnumber of bytes to dump\n");
	printf("\t--help\t display this help and exit\n");
	return 0;
    }

    /*------------------*/
    /* Process options. */
    /*------------------*/
    while (nargs > 1 && args[1][0] == '-') {
	if (!strcmp(args[1],"-f")) {
	    force = TRUE;
	    nargs--;
	    args++;
	    continue;
	}
	printf("Unknown option: %s\n", args[1]);
	printf("Try %s --help for more information.\n", args[0]);
	return 0;
    }

    if (force) {
	printf("-f option is currently useless, and is ignored.\n");
    }

    /*-------------------------------------*/
    /* Check for valid number of arguments */
    /*-------------------------------------*/
    if (nargs != 3) {
	printf("%s: incorrect number of arguments.\n", args[0]);
	printf("Try %s --help for more information.\n", args[0]);
	return 0;
    }

	start = parseval(args[1]);
	if (err) {
	    fprintf(stderr, "%s: invalid starting address\n", args[1]);
	    return 1;
	}

	length = parseval(args[2]);
	if (err) {
	    fprintf(stderr, "%s: invalid length\n", args[2]);
	    return 1;
	}

	/* Round starting address down to multiple of four and round	*/
	/*	length up to a multiple of four				*/

	start &= ~0x3;
	length = (length + 3) & ~0x3;

	/* verify that the address and length are reasonable */
	/* XXX This is questionable. Omit the test for now.  */

	/* Values are valid; perform dump. */

	chptr = (char *)start;
	for (l=0; l<length; l+=16) {
	    printf("%08x: ", start);
	    addr = (uint32 *)start;
	    for (i=0; i<4; i++) {
		printf("%08x ",*addr++);
	    }
	    printf("  *");
	    for (i=0; i<16; i++) {
		ch = *chptr++;
		if ( (ch >= 0x20) && (ch <= 0x7e) ) {
		    printf("%c",ch);
		} else {
		    printf(".");
		}
	    }
	    printf("*\n");
	    start += 16;
	}
	return 0;
}

/*------------------------------------------------------------------------
 * parse - parse an argument that's octal, decimal, or hex.
 *------------------------------------------------------------------------
 */
static	uint32	parseval(
	  const char *s			/* argument string to parse	*/
	)
{
	unsigned long v;		/* result of strtoul		*/
	char *np;			/* endpointer from strtoul	*/

	v = strtoul(s, &np, 0);		/* try to convert the value	*/

	if (v == 0 && np == s) {	/* if no digits processed	*/
	    err = 1;
	    return 0;
	}

	while (*np == ' ')		/* skip any trailing blanks	*/
	    np++;

	if (*np == '\0') {		/* processed the whole string?	*/
	    if (v <= UINT_MAX) {	/* result ok?			*/
		err = 0;
		return (unsigned int)v;	/* success			*/
	    }
	}
	err = 1;
	return 0;
}
