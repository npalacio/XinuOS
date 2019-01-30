/*--------------------------------------------------------------------------*/
/* We will likely remove the current mechanism used to deal with command    */
/* line arguments. But we will provide some mechanism that can be used by   */
/* a program invoked with something like "basic filename ..." to handle     */
/* command line arguments.                                                  */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* basic - a BASIC interpreter for the Xinu operating system		    */
/*									    */
/* When changes are made, change the version number, add an appropriate     */
/* comment (after the previous version's comment), and change the global    */
/* variable 'version'.                                                      */
/*									    */
/* Version 1.0 (?) This version was for Tempo				    */
/* Version 2.0 (September 23, 2015)					    */
/* Version 2.1 (October 15, 2016)					    */
/*	Added MEMCHECK to allow checking for memory allocation problems     */
/*	Corrected a bug with freeing storage for a command line argument    */
/* Version 2.2 (November 7, 2016)					    */
/*	Removed the BEEP statement					    */
/*	Removed the DOS statement					    */
/*	Renamed to "basic" (previously basicx)				    */
/* Version 2.3 (November 13, 2016)					    */
/*	Added the STRLEN function.					    */
/*      Arranged for EOF on string INPUT to return an empty string	    */
/* Version 2.4 (February 14, 2017)					    */
/*	Added a little documentation, and noted a few "todo" items.	    */
/*									    */
/* Version 2.5 (February 15, 2017)					    */
/*	Added the "IN1", "IN2" and "IN4" functions. The argument is an I/O  */
/*	port number which must be in the range 0 to 65535. If not, the      */
/*	execution is stopped with an appropriate error message. Otherwise   */
/*	the appropriate number of bytes is read from the port and returned. */
/*	For IN1 and IN2, the high-order bytes of the result are always 0.   */
/*									    */
/*	Added the "OUT1", "OUT2", and "OUT4" statements. The general form   */
/*	is "OUTx   port#, value". port# must be in 0..65535. Value can be   */
/*	any value, but only the low-order 8 or 16 bits are used for the     */
/*	OUT1 and OUT2 statements.					    */
/*									    */
/*	Added the "HEX$" function which returns an 8-byte string with the   */
/*	hexadecimal representation of the numeric argument.		    */
/*									    */
/*	Added recognition of octal and hexadecimal constants (in addition   */
/*	to decimal constants). Modified code to require a decimal line      */
/*	number instead of octal or hex line number.			    */
/*									    */
/* Version 2.6 (August 23, 2017)					    */
/*	No significant changes, but did a little cleanup of documentation.  */
/*									    */
/* Variables:								    */
/*	260 Integer variables	:	A0-A9 ... Z0-Z9			    */
/*	260 Character variables	:	A0$-A9$ ... Z0$-Z9$		    */
/*	260 Integer arrays	:	A0()-A9() ... Z0()-Z9()		    */
/*									    */
/*	For convenience the '0' variables can be referenced by letter	    */
/*	only. That is, A is equivalent to A0, and Z$ is equivalent to Z0$   */
/*									    */
/* Statements:								    */
/*	CLEAR				- Erase variables only		    */
/*	CLOSE#n				- Close file (0-9) opened with OPEN */
/*	DATA				- Enter "inline" data statements    */
/*	DELAY ms			- Delays for ms milliseconds	    */
/*	DIM var(size)[, ... ]		- Dimension an array		    */
/*	END				- Terminate program with no message */
/*	EXIT				- Terminate prog; return to shell   */
/*	FOR v=init TO limit [STEP increment] - Perform a counted loop	    */
/*	GOSUB line			- Call a subroutine		    */
/*	GOTO  line			- Jump to line			    */
/*	IF test THEN line		- Conditional goto		    */
/*	IF test THEN statement	- Conditional stmt (next statement only)    */
/*	INPUT var			- Get value for variable	    */
/*	INPUT "prompt",var		- Get value of variable with prompt */
/*		prompt must be a constant string.			    */
/*		However you can use a char variable in "prompt" by	    */
/*		concatenating it to such a string: INPUT ""+a$,b$	    */
/*	INPUT#n,var			- Get variable value from file 0-9  */
/*	LET (default)			- variable = expression		    */
/*	LIF test THEN statements- LONG IF (all statements to end of line)   */
/*	LIST [start,[end]]		- List program lines		    */
/*	LIST#n ...			- List program to file 0-9	    */
/*	LOAD "name"			- Load program from disk file	    */
/*		When LOAD is used within a program, execution continues     */
/*		with the first line of the newly loaded program. In this    */
/*		case, the user variables are not cleared. This provides     */
/*		a means of chaining to a new program, and passing	    */
/*		information to it. Also note that load must be the last     */
/*		statement on a line.					    */
/*	NEW				- Erase program and variables       */
/*	NEXT [v]			- End counted loop		    */
/*	OPEN#n,"name","mode"		- Open file 0-9			    */
/*					  mode:				    */
/*					      r=read, w=write, neither=rw   */
/*					      o = old (must already exist)  */
/*					      n = new (must NOT exist)	    */
/*					  				    */
/*	ORDER line			- Position data read pointer	    */
/*									    */
/*      XXX WE MIGHT WANT TO CHANGE THE NAME TO SOMETHING LIKE "STORE"	    */
/*	XXX OUT addr,expr		- store a byte in a memory location */
/*									    */
/*	OUT1 port,expr			- Write 1 byte to an I/O port	    */
/*	OUT2 port,expr			- Write 2 bytes to an I/O port	    */
/*	OUT4 port,expr			- Write 4 bytes to an I/O port	    */
/*									    */
/*	PRINT expr[,expr ...]		- Print to console		    */
/*	PRINT#n,...			- Print to file 0-9		    */
/*	READ var[,var ...]		- Read data from program statements */
/*		You MUST issue an "ORDER" statement targeting a line	    */
/*		containing a valid DATA statement before using READ	    */
/*	RETURN				- Return from subroutine	    */
/*	REM				- Comment... rest of line ignored   */
/*	RUN [line]			- Run program			    */
/*	SAVE ["name"]			- Save program to named file	    */
/*	STOP				- Terminate program & issue message */
/*									    */
/* Operators:								    */
/*	+				- Addition, string concatenation    */
/*	-				- Unary minus, subtraction	    */
/*	*, /, %,			- multiplication, division, modulus */
/*	&, |, ^				- binary AND, OR, Exclusive OR	    */
/*	=				- Assign or test equality	    */
/*	<>				- test inequality (num or string)   */
/*	<, <=, >, >=			- LT, LE, GT, GE (numbers only)     */
/*	!				- Unary NOT (one's complement)	    */
/*		The "test" operators (=, <>, <, <=, >, >=) can be used in   */
/*		expressions, and evaluate to 1 if TRUE, and 0 if FALSE. The */
/*		IF and LIF commands treat any non-zero value as TRUE.	    */
/*									    */
/* Functions:								    */
/*	CHR$(value)			- Returns character of passed value */
/*	STR$(value)			- Returns ASCII string of value     */
/*	ASC(char)			- Returns value of passed character */
/*	NUM(string)			- Convert string to number	    */
/*	ABS(value)			- Returns absolute value of 'value' */
/*	RND(value)			- Random number from 0 to (value-1) */
/*	STRLEN(string)			- Returns length of string	    */
/*	MEM(addr)			- Return byte at location 'addr'    */
/*	IN1(port)			- Read one byte from 'port'	    */
/*	IN2(port)			- Read two bytes from 'port'	    */
/*	IN4(port)			- Read four bytes from 'port'	    */
/*	HEX$(value)			- Return string value as hex number */
/*--------------------------------------------------------------------------*/

/* To do...
 *
 *    Line numbers (currently) can include 0 (and possibly negative values?).
 *    Restrict line numbers to a reasonable range (1..2147483647?). Also
 *    eliminate the flawed treatment of line numbers in other bases.
 *
 *    Numeric constant overflow is detected assuming integers are unsigned.
 *    But most integer constants are probably intended to be signed (but the
 *    interpreter doesn't allow the specification of unsigned values). What
 *    should be done with this?
 *
 *    For consistency, should be also have an OCT$ function?
 *
 *    Document array subscripting. In particular are they 0 or 1 origin?
 *    There is also a flaw in the subscript checking, especially on
 *    assignment. This should be corrected.
 *
 *    There may still be some 16-bit stuff around. Seek out and destroy!
 *
 *    Add a facility that allows a program to be interrupted or aborted.
 *    Logically this might be associated with the pressing of something
 *    like the control-C key. Of course we'd then need to be in raw mode.
 *
 *    Complete the integration with Xinu files.
 *
 *    Document the system in more detail. In particular, we need to document
 *    the invocation of the interpreter, and how it deals with command line
 *    arguments. The various commands that deal with entire programs need to
 *    be documents (e.g. LOAD, SAVE, RUN)
 *
 *    Document (and perhaps change) the formatting that is performed by the
 *    PRINT statement.
 *
 *    Prepare a reasonably large set of sample programs to illustrate the
 *    use of the system.
 *
 *    Add additional string manipulation facilities.
 *
 *    Add new, or modify existing, functions and statements to support
 *    consistent access to memory and I/O ports. The goal is to be able to
 *    use BASIC to do I/O device fiddling.
 *
 *    Perhaps add a more sophisticated mechanism to deal with time. Perhaps
 *    allow timers to be created with associated expiration callouts. Also
 *    allow timers to be queried (remaining time), reset, and deleted.
 *
 *    Carefully document the method for adding or changing statements.
 *    Document the internal program structure.
 *
 * Notes.
 *
 *    To deal with extraneous information in a statement, look at the code
 *    for OUT1, OUT2, and OUT4. After getting all the relevant pieces, then
 *    do "is_l_end(skip_blank())" This should return 1. If not, there is
 *    more stuff -- junk!
 */

#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <setjmp.h>

#define MAXFILES 10			/* limit on number of open files */
#define MAXFILENAME 65			/* maximum filename characters */

#define MEMCHECK			/* define to enable malloc checks */

/*----------------------------------*/
/* malloc and free - local versions */
/*----------------------------------*/

/*----------------------------------------------------------------------*/
/* This malloc implementation saves the size of the allocated region in */
/* the uint32 that immediately precedes the address of the region given */
/* to the caller. The free function can then determine the size to be   */
/* used with the freemem function.                                      */
/*----------------------------------------------------------------------*/
/* If MEMCHECK is define, we also allocate an extra two bytes as a flag */
/* to allow free to verify, to some extent, that the pointer that was   */
/* given to it really identifies some storage that was allocated by     */
/* malloc.                                                              */
/*----------------------------------------------------------------------*/
/* Later we'll add another more rigorous check by keeping track of each */
/* allocation in a list. That'll allow us to verify that all allocated  */
/* memory is deallocated when the program terminates.                   */
/*----------------------------------------------------------------------*/
#define MALLOC_FLAG 0xD6F2		/* hopefully reasonably unique */

static void *malloc(uint32 n)
{
    char *x;

    if (n == 0)				/* special case */
	return NULL;

#ifdef MEMCHECK
    x = (char *)getmem(n + sizeof(uint32) + sizeof(uint16));
#else
    x = (char *)getmem(n + sizeof(uint32)); /* get n + sizeof(uint32) bytes */
#endif

    if (x == (char *)SYSERR)		/* if allocation fails */
	return NULL;

    *(uint32 *)x = n;			/* save size of requested allocation */
#ifdef MEMCHECK
    *(uint16 *)(x + sizeof(uint32)) = MALLOC_FLAG;	/* save MALLOC flag */
    return (void *)(x + sizeof(uint32) + sizeof(uint16));
#else
    return (void *)(x + sizeof(uint32));
#endif

}

static void free(void *p)
{
    char *x;
    uint32 n;

    if (p == NULL) {			/* sanity check */
	printf("Trying to free a NULL pointer!\n");
	exit();
    }

#ifdef MEMCHECK
    x = (char *)p - sizeof(uint32) - sizeof(uint16);
    n = sizeof(uint32) + sizeof(uint16) + *(uint32 *)x;
    if (*(uint16 *)(x + sizeof(uint32)) != MALLOC_FLAG) {
	printf("ERROR: TRYING TO FREE STORAGE NOT ALLOCATED BY MALLOC.\n");
	exit();
    }
#else
    x = (char *)p - sizeof(uint32);	/* point to the real allocation */
    n = sizeof(uint32) + *(uint32 *)x;	/* get the real allocation size */
#endif
    if (freemem(x,n) == SYSERR) {
	printf("freemem failed; errno = %d\n", proctab[currpid].errno);
	exit();
    }
}

/*----------------------------------------------------------------*/
/* Make a copy of the null-terminated string pointed to by s by   */
/* allocating storage for the copy. Return a pointer to the copy. */
/* On allocation failure, display a message and exit.             */
/*----------------------------------------------------------------*/
static char *strdup(char *s)
{
    char *p;			/* new allocation */

    if (s == NULL) {
	printf("NULL argument to strdup.\n");
	exit();
    }
    p = malloc(strlen(s)+1);	/* allocate the memory */
    if (p == NULL) {
	printf("Out of memory (strdup).\n");
	exit();
    }
    strcpy(p,s);
    return p;
}

/***************************************************/
/* Function prototypes (forward declarations only) */
/***************************************************/
static void disp_pgm(uint32, uint32, uint32);
static void pgm_only(void);
static void skip_stmt(void);
static void error(unsigned);
static void eval_char(void);
static int32 eval(void);
static void get_char_value(char *);
static void num_string(unsigned, char *);
static void hex_string(unsigned, char *);
static void clear_pgm(void);
static void clear_vars(void);
static uint32 get_var(void);
static int32 eval_sub(void);
static int32 eval_num(void);
static int32 get_value(void);
static uint32 chk_file(char);
static int do_arith(char opr, unsigned op1, unsigned op2);

static int32 random(int limit);

/*---------------------------------------------------------------------*/
/* The rcsid was previously here, and the version number was extracted */
/* from it and displayed when the interpreter started. That's been     */
/* removed and replaced with just version information, and this is     */
/* only displayed when the command is given without arguments.         */
/*---------------------------------------------------------------------*/
static char *version = "Xinu BASIC v 2.6\n";

/*------------------*/
/* Fixed parameters */
/*------------------*/
#define BUFFER_SIZE 100		/* input buffer size */
#define NUM_VAR 260		/* number of variables */
#define SA_SIZE 100		/* string accumulator size */

/*------------------------------------*/
/* Control stack constant identifiers */
/*------------------------------------*/
#define _FOR	1000	/* indicate FOR statement */
#define _GOSUB	_FOR+1	/* indicate GOSUB statement */

/*------------------*/
/* Primary keywords */
/*------------------*/
#define	LET	1
#define	EXIT	2
#define	LIST	3
#define	NEW	4
#define	RUN	5
#define	CLEAR	6
#define	GOSUB	7
#define	GOTO	8
#define	RETURN	9
#define	PRINT	10
#define	FOR	11
#define	NEXT	12
#define	IF	13
#define	LIF	14
#define	REM	15
#define	STOP	16
#define	END	17
#define	INPUT	18
#define	OPEN	19
#define	CLOSE	20
#define	DIM	21
#define	ORDER	22
#define	READ	23
#define	DATA	24
#define	SAVE	25
#define	LOAD	26
#define	XDELAY	27
#define	OUT1	28
#define	OUT2	29
#define	OUT4	30

/*--------------------*/
/* Secondary keywords */
/*--------------------*/
#define	TO	31		/* Also used as marker */
#define	STEP	32
#define	THEN	33

/*-------------------------*/
/* Operators and functions */
/*-------------------------*/
#define	ADD	34		/* Also used as marker */
#define	SUB	35
#define	MUL	36
#define	DIV	37
#define	MOD	38
#define	AND	39
#define	OR	40
#define	XOR	41
#define	EQ	42
#define	NE	43
#define	LE	44
#define	LT	45
#define	GE	46
#define	GT	47
#define	CHR	48
#define	STR	49
#define	ASC	50
#define	ABS	51
#define	NUM	52
#define	RND	53
#define	STRLEN	54
#define	MEM	55
#define IN1	56
#define IN2	57
#define IN4	58
#define HEX	59

#define	RUN1	99

#if 0
/*----------------------------------------------------------------------*/
/* Allocation observation. External variables that are initialized to a */
/* non-zero value are placed in the initialized data segment. Storage   */
/* for xp and xp2 fit this pattern. It's interesting to note that xp is */
/* stored at the address that is one byte LARGER than xp2, which is not */
/* what you'd expect.                                                   */
/* The variable xp3, even though it is explicitly initialized, is set   */
/* to zero. This is what would happen normally for variables in the bss */
/* region of memory. So xp3 is NOT adjacent to xp or xp2.               */
/*----------------------------------------------------------------------*/
static char xp = 27;			/* EXPERIMENT */
static char xp2 = 1;			/* MORE EXPERIMENT */
static char xp3 = 0;			/* MORE EXPERIMENT */
#endif

/*-----------------------------------------------------------------*/
/* This table must match the order of the definitions given above. */
/*-----------------------------------------------------------------*/
static char *reserved_words[] = {
	"LET", 			/* 1 */
	"EXIT", 		/* 2 */
	"LIST", 		/* 3 */
	"NEW", 			/* 4 */
	"RUN", 			/* 5 */
	"CLEAR", 		/* 6 */
	"GOSUB", 		/* 7 */
	"GOTO",			/* 8 */
	"RETURN", 		/* 9 */
	"PRINT", 		/* 10 */
	"FOR", 			/* 11 */
	"NEXT", 		/* 12 */
	"IF", 			/* 13 */
	"LIF", 			/* 14 */
	"REM", 			/* 15 */
	"STOP",			/* 16 */
	"END", 			/* 17 */
	"INPUT", 		/* 18 */
	"OPEN", 		/* 19 */
	"CLOSE", 		/* 20 */
	"DIM", 			/* 21 */
	"ORDER", 		/* 22 */
	"READ", 		/* 23 */
	"DATA",			/* 24 */
	"SAVE", 		/* 25 */
	"LOAD", 		/* 26 */
	"DELAY", 		/* 27 */
	"OUT1", 		/* 28 */
	"OUT2", 		/* 29 */
	"OUT4",			/* 30 */
	"TO", 			/* 31 */
	"STEP", 		/* 32 */
	"THEN",			/* 33 */
	"+", 			/* 34 */
	"-", 			/* 35 */
	"*", 			/* 36 */
	"/", 			/* 37 */
	"%", 			/* 38 */
	"&", 			/* 39 */
	"|", 			/* 40 */
	"^", 			/* 41 */
	"=",  			/* 42 */
	"<>",  			/* 43 */
	"<=",  			/* 44 */
	"<",  			/* 45 */
	">=",  			/* 46 */
	">", 			/* 47 */
	"CHR$(", 		/* 48 */
	"STR$(", 		/* 49 */
	"ASC(", 		/* 50 */
	"ABS(", 		/* 51 */
	"NUM(", 		/* 52 */
	"RND(", 		/* 53 */
	"STRLEN(", 		/* 54 */
	"MEM(",			/* 55 */
	"IN1(",			/* 56 */
	"IN2(",			/* 57 */
	"IN4(",			/* 58 */
	"HEX$(",		/* 59 */
	0 };

/*---------------------*/
/* Operator priorities */
/*---------------------*/
static char priority[] = { 0, 1, 1, 2, 2, 2, 3, 3, 3, 1, 1, 1, 1, 1, 1 };

/*----------------*/
/* Error messages */
/*----------------*/
static char *error_messages[] = {	/* Error number */
    "Syntax",				/* 0 */
    "Illegal program",			/* 1 */
    "Illegal direct",			/* 2 */
    "Line number",			/* 3 */
    "Wrong type",			/* 4 */
    "Divide by zero",			/* 5 */
    "Nesting",				/* 6 */
    "File not open",			/* 7 */
    "File already open",		/* 8 */
    "Input",				/* 9 */
    "Dimension error",			/* 10 */
    "Data",				/* 11 */
    "Out of memory",			/* 12 */
    "File not found",			/* 13 */
    "Invalid port number",		/* 14 */
    "Bad numeric constant",		/* 15 */
    "Subscript error",			/* 16 */
    "DEBUG"				/* 17 */
};

/*------------------------------------------------------------*/
/* The "program" is a singly-linked list of these structures. */
/*------------------------------------------------------------*/
struct line_record {
    uint32 Lnumber;			/* the line number */
    struct line_record *Llink;		/* ptr to the next line */
    char Ltext[1];			/* null-terminated line */
};

static char sa1[SA_SIZE], sa2[SA_SIZE];		/* String accumulators */
static struct line_record *pgm_start,		/* Indicates start of program */
    *runptr,					/* Line we are RUNnning */
    *readptr;					/* Line we are READing */

static int32 prog_mode;				/* 0 = prog, 1 = interactive */

static uint32 dim_check[NUM_VAR];		/* Check dim sizes for arrays */

static uint32 filein, fileout;			/* File I/O active pointers */

jmp_buf savjmp;					/* setjmp/longjmp save area */

/*--------------------------------*/
/* Miscellaneous global variables */
/*--------------------------------*/
static char *cmdptr,				/* Command line parse pointer */
    *dataptr,					/* Read data pointer */
    buffer[BUFFER_SIZE],			/* General input buffer */
    mode,					/* 0=Stopped, !0=Running */
    expr_type,					/* Type of last expression */
						    /* XXX DOCUMENT THIS */
    nest;					/* Nest level of expr. parser */
static uint32 line,				/* Current line number */
    ctl_ptr,					/* Control stack pointer */
    ctl_stk[50];				/* Control stack */

/*------------------------------------------------------*/
/* The following variables must be initialized to zero. */
/* For safety, they are explicitly initialied by the    */
/* xsh_basic function (with which execution begins).    */
/*------------------------------------------------------*/
static char filename[MAXFILENAME];		/* Name of program file */
static uint32 files[MAXFILES];			/* File unit nums = Xinu Devs */
						/* 0 means "not open" */
static int32 num_vars[NUM_VAR];			/* Numeric variables */
static int32 *dim_vars[NUM_VAR];		/* Dimensioned arrays */
static char *char_vars[NUM_VAR];		/* Character variables */

/*------------------------------------------------------------------*/
/* Free any storage that was dynamically allocated during execution */
/* but not already returned.                                        */
/*------------------------------------------------------------------*/
static void reclaim(void)
{
    clear_vars();
    clear_pgm();
    /* XXX more to be written? */
    /* XXX handle the closing of any open files here? */
}

/*-------------------------------------------------*/
/* Initialize all the variables to 0 or undefined. */
/* This is called by xsh_basic at program start.   */
/*-------------------------------------------------*/
static void init_vars(void)
{
    uint32 i;

    filename[0] = '\0';
    for(i=0;i<MAXFILES;i++)			/* mark files not open */
	files[i] = 0;
    for(i=0;i<NUM_VAR;i++) {
	num_vars[i] = 0;
	dim_vars[i] = (int32 *)NULL;
	char_vars[i] = (char *)NULL;
    }
}

/*-----------------------------*/
/* Sleep for 'ms' milliseconds */
/*-----------------------------*/
static void delay(int ms)
{
    sleepms(ms);
}

/********************************/
/* Read from memory location m. */
/********************************/
static unsigned rdmem(int32 m)
{
    unsigned char *p;

    if (m < 0)			/* Hmm. What should we really do here? */
	return 0;		/* Perhaps we should fail with a msg?  */

    p = (unsigned char *)m;
    return (unsigned)(*p);
}

/*----------------------------------------------------------------*/
/* Generate a random number between 0 and 'limit'-1.              */
/* This is an ad hoc implementation. We generate a random number  */
/* between 0 and 0x7fffffff from three invocations of rand, each  */
/* invocations yielding a random number in the range 0 to 0x7fff. */
/* We then return that number mod limit.                          */
/*----------------------------------------------------------------*/
static int32 random(int limit)
{
    int32 x;

    if (limit < 2)			/* handle weird arguments */
	return 1;

    x = (rand() << 16) ^ (rand() << 8) ^ rand();
    /*   0x7fff0000        0x7fff00      0x7fff   <-- limiting values */
    return x % limit;
}

/*-----------------------------------*/
/* Concatenate s1 and s2 to yield s. */
/*-----------------------------------*/
static void concat(char *s, char *s1, char *s2)
{
    strcpy(s,s1);
    strcat(s,s2);
}

/*--------------------------------------------------*/
/* Predicate: does c mark the end of an expression? */
/*--------------------------------------------------*/
static int is_e_end(char c)
{
    if ((c >= (-128+TO)) && (c < (-128+ADD))) return(1);
    return (c == '\0') || (c == ':') || (c == ')') || (c == ',');
}

/*------------------------------------------------*/
/* Predicate: does c mark the end of a statement? */
/*------------------------------------------------*/
static int is_l_end(char c)
{
    return (c == '\0') || (c == ':');
}

/*-----------------------------------*/
/* Predicate: is c a blank or a tab? */
/*-----------------------------------*/
static uint32 isterm(char c)
{
    return (c == ' ') || (c == '\t');
}

/*--------------------------------------------*/
/* Advance cmdptr until it points to the next */
/* non-blank/non-tab character and return it. */
/*--------------------------------------------*/
static char skip_blank(void)
{
    while(isterm(*cmdptr))
	cmdptr++;
    return *cmdptr;
}

/*----------------------------------------------*/
/* Advance cmdptr to the next non-blank/non-tab */
/* and return it. If it is not the null byte at */
/* the end of a line, then move cmdptr past it. */
/*----------------------------------------------*/
static char get_next(void)
{
    char c;

    c = *cmdptr;
    while (isterm(c)) {
	cmdptr++;
	c = *cmdptr;
    }
    if (c)
	cmdptr++;
    return c;
}

/*------------------------------------------------------------*/
/* Check for a specific token appearing as the next non-blank */
/* in the statement. If so, skip over it and return -1.       */
/* If it is NOT the expected token, then return 0.            */
/*------------------------------------------------------------*/
static int test_next(int token)
{
    if (skip_blank() == token) {
	++cmdptr;
	return -1;
    }
    return 0;
}

/*----------------------------------------------------------------*/
/* Expect a specific token - generate a syntax error if not found */
/*----------------------------------------------------------------*/
static void expect(uint32 token)
{
    if (get_next() != token)
	error(0);
}

/*-----------------------------------------------------------------*/
/* If the word starting a 'cmdptr' matches a word in 'table', then */
/* return the 1-origin table index. Otherwise return 0.            */
/*-----------------------------------------------------------------*/
#if EXSYM
uint32 lookup(char *table[])
#else
static uint32 lookup(char *table[])
#endif
{
    uint32 i;
    char *cptr;			/* pointer to a word in the table */
    char *optr;			/* original command pointer */

    optr = cmdptr;

    /*---------------------------------------------------------------------*/
    /* Determine if the characters starting at the position pointed to by  */
    /* cmdptr match all of the characters in any string in table, ignoring */
    /* case. The end of each string must be a non-alphanumeric character.  */
    /* If such is the case, advance cmdptr beyond the blanks following     */
    /* the matched string, and return the index of the matched string in   */
    /* the table. If there is no match return 0, and don't change cmdptr.  */
    /*---------------------------------------------------------------------*/
    for (i=0; (cptr = table[i]); ++i) {

	/*----------------------------------------------------------*/
	/* Match as many characters as possible with a table entry. */
	/*----------------------------------------------------------*/
	while((*cptr) && (*cptr == toupper(*cmdptr))) {
	    ++cptr;
	    ++cmdptr;
	}

	/*--------------------------------------------------*/
	/* If we matched all characters in the table entry, */
	/* and if it did not end with a letter or digit.    */
	/*--------------------------------------------------*/
	if (!*cptr) {
	    if(!(isalnum(*(cptr-1)) && isalnum(*cmdptr)) ) {
		skip_blank();
		return i+1;
	    }
	}
	cmdptr = optr;
    }
    return 0;
}

/*---------------------------------------------*/
/* Get a decimal number from the input buffer. */
/* Report a syntax error if there isn't one.   */
/*---------------------------------------------*/
static uint32 get_dnum(void)
{
    uint32 value;			/* value of the number */
    uint32 value10;			/* value * 10 */
    int32 got1;				/* nonzero if we've got a digit */
    char c;				/* current input character */

    value = got1 = 0;
    for(;;) {
	c = *cmdptr;
	if (!isdigit(c))
	    break;
	value10 = value * 10;
	if (value10 / 10 != value)
	    error(15);
	value = value10 + c - '0';
	if ((value < value10) || (value < c - '0'))
	    error(15);
	cmdptr++;
	got1 = 1;
    }
    if (!got1)
	error(3);
    return value;
}

/*----------------------------------------------------------------------*/
/* Get the value of an unsigned numeric constant from the input buffer. */
/* Decimal values must begin with '1'..'9'. An octal value must begin   */
/* with '0' and not be followed by 'x' or 'X'. A hexadecimal value must */
/* begin with "0x" or "0X" and be followed by at least one more digit.  */
/* On error...								*/
/*----------------------------------------------------------------------*/
static uint32 get_num(void)
{
    uint32 base;			/* 8, 10, or 16 */
    uint32 value;			/* the result */
    uint32 valueb;			/* value * base */
    char c;				/* current input character */
    uint32 cvalue;			/* current character's value */
    int32 got1 = 0;			/* did we get at least one digit? */

    base = 10;
    c = *cmdptr;
    if (c >= '1' && c <= '9')
	base = 10;
    else if (c == '0') {
	base = 8;
	cmdptr++;
	if (*cmdptr == 'x' || *cmdptr == 'X') {
	    cmdptr++;
	    base = 16;
	} else
	    got1 = 1;			/* single octal digit of 0 */
    } else
	error(0);

    value = 0;
    for(;;) {
	if (*cmdptr >= '0' && *cmdptr <= '9')
	    cvalue = *cmdptr - '0';
	else if (*cmdptr >= 'a' && *cmdptr <= 'f')
	    cvalue = *cmdptr - 'a' + 10;
	else if (*cmdptr >= 'A' && *cmdptr <= 'F')
	    cvalue = *cmdptr - 'A' + 10;
	else
	    break;
	if (cvalue >= base)
	    error(0);
	got1 = 1;
	valueb = value * base;
	if (valueb / base != value)
	    error(15);
	value = valueb + cvalue;
	if (value < valueb || value < cvalue)
	    error(15);
	cmdptr++;
    }
    if (!got1)
	error(0);
    return value;
}

/*******************************/
/* Allocate memory and zero it */
/*******************************/
static char *allocate(uint32 size)
{
    char *ptr;

    ptr = (char *)malloc(size);
    if (ptr == NULL)
	error(12);
    memset(ptr, 0, size);
    return ptr;
}

/*---------------------------------------------------*/
/* If a line with the number 'lino' exists in the    */
/* program, delete it. Otherwise ignore the request. */
/*---------------------------------------------------*/
/* XXX This could be improved a little by noting     */
/*     that the nodes are ordered by Lnumber.        */
/*     So after cptr->Lnumber is greater than lino,  */
/*     we know the specified line does not exist.    */
/*---------------------------------------------------*/
#if EXSYM
void delete_line(uint32 lino)
#else
static void delete_line(uint32 lino)
#endif
{
    struct line_record *cptr;			/* ptr to current line */
    struct line_record *bptr;			/* ptr to previous line */

    cptr = pgm_start;				/* first line */
    bptr = NULL;				/* no previous line */

    while(cptr != NULL) {			/* while lines are left */
	if (lino == cptr->Lnumber) {		/* match? */
	    if (cptr == pgm_start) {		/* first line? */
		pgm_start = cptr->Llink;	/* yes; replace head ptr */
	    } else {
	        bptr->Llink = cptr->Llink;	/* no; replace prev line */
	    }
	    free(cptr);				/* release line's storage */
	    return;
	}
	bptr = cptr;
	cptr = cptr->Llink;
    }
}

/*---------------------------------*/
/* Insert a line into the program. */
/*---------------------------------*/
#if EXSYM
void insert_line(uint32 lino)
#else
static void insert_line(uint32 lino)
#endif
{
    uint32 i, len;
    struct line_record *bptr;		/* ptr to new node */
    struct line_record *cptr;		/* ptr to current node */
    struct line_record *pptr;		/* ptr to previous node */

    /*----------------------------------------*/
    /* Allocate storage for the new line.     */
    /* If allocation fails, report the error. */
    /*----------------------------------------*/
    len = strlen(cmdptr) + 1;				/* length */
    bptr = (struct line_record *)malloc(sizeof(struct line_record) + len);
    if (bptr == NULL)
	error(12);


    /*-------------------------*/
    /* Initialize the storage. */
    /*-------------------------*/
    bptr->Lnumber = lino;
    for(i=0; *cmdptr; ++i)
	bptr->Ltext[i] = *cmdptr++;
    bptr->Ltext[i] = 0;

    /*------------------------------------------------------*/
    /* Set pptr/cptr to nodes before/after insertion point. */
    /*------------------------------------------------------*/
    pptr = NULL;
    cptr = pgm_start;

    for(;;) {
	if (cptr == NULL)				/* end of program */
	    break;
	if (lino < cptr->Lnumber)			/* new first stmt */
	    break;
	pptr = cptr;
	cptr = cptr->Llink;
    }

    bptr->Llink = cptr;					/* set next link */
    if (pptr == NULL)					/* adjust prev node */
	pgm_start = bptr;
    else
	pptr->Llink = bptr;
}

/*--------------------------------------------------------------------*/
/* Do some lexical analysis on an input line (specifically, recognize */
/* keywords and replace them by a byte greater than 0x80). If the     */
/* line starts with a number -- that is, a line number -- add it to   */
/* the saved program text, replacing any previous line with the same  */
/* number, and return -1. Otherwise the line is assumed to be an      */
/* immediate command, so return 0.				      */
/*--------------------------------------------------------------------*/
#if EXSYM
int edit_program()
#else
static int edit_program()
#endif
{
    uint32 value, len;
    char *ptr, c;

    /*-------------------------------------------*/
    /* Strip end of line from buffer, if present */
    /*-------------------------------------------*/
    len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n')
	buffer[len-1] = '\0';

    cmdptr = buffer;
    ptr = buffer;

    /*-------------------------------------*/
    /* Translate special tokens into codes */
    /*-------------------------------------*/
    while ((c = *cmdptr) != 0) {
	value = lookup(reserved_words);
	if (value) {			/* value != 0 if recognized token */
	    *ptr++ = 0x80 | value;
	} else {
	    *ptr++ = c;
	    cmdptr++;

	    /*------------------------------------------------*/
	    /* Accumulate a string enclosed in double quotes. */
	    /*------------------------------------------------*/
	    if (c == '"') {		/* double quote */
		while((c = *cmdptr) && (c != '"')) {
		    cmdptr++;
		    *ptr++ = c;
		}
		*ptr++ = *cmdptr++;
	    }
	}
    }

    *ptr = 0;
    cmdptr = buffer;

    if (isdigit(skip_blank())) {	/* Valid source line */
	value = get_dnum();		/* Get line number */
	delete_line(value);		/* Delete the old line, if any */
	if(skip_blank())		/* Insert the new line */
	    insert_line(value);
	return -1;
    }
    return 0;
}

/*********************************************************/
/* Locate the line with line number 'line' in the source */
/*********************************************************/
static struct line_record *find_line(unsigned line)
{
    struct line_record *cptr;

    for (cptr = pgm_start; cptr; cptr = cptr->Llink)
	if (cptr->Lnumber == line) return cptr;
    error(3);
    return 0;	/* XXX This really isn't reached */
}

/*------------------------------------------*/
/* Compute variable address for assignment. */
/* Set expr_type to indicate the type of    */
/* variable encountered.                    */
/*------------------------------------------*/
static uint32 *address(void)
{
    uint32 i, j;
    uint32 *dptr;

    i = get_var();			/* get variable index & type */
    if (expr_type) {			/* if it's a char string */
	return (uint32 *)&char_vars[i];		/* yes */

    } else {				/* nope, must be integer */
	if (test_next('(')) {		/* Is it subscripted? */

	    if (expr_type)		/* Is this even possible? */
		error(0);		/* I think not. */

	    dptr = (uint32 *)dim_vars[i];
	    if (dptr == NULL) {
		error(17);		/* DEBUG */
		error(10);		/* undimensioned array */
	    }

	    nest = 0;
	    j = eval_sub();		/* evaluate subscript */
	    if (j >= dim_check[i])	/* check for subscript error */
		error(16);
	    if (j < 0)
		error(16);
	    return &dptr[j];		/* return address of target var */
	}
    }

    return (uint32 *)&num_vars[i];	/* return integer variable's address */
}

/*-------------------------*/
/* Execute a BASIC command */
/*-------------------------*/
static struct line_record *execute(char cmd)
{
    uint32 i, j, k, *dptr;
    int32 ii, jj;
    int32 port;				/* port number for out1, out2, out4 */
    int32 syscr;			/* system call result */
    struct line_record *cptr;
    char c;

    switch(cmd & 0x7F) {

	case LET:
	    dptr = address();
	    j = expr_type;

	    expect(-128+EQ);

	    k = eval();

	    if (j != expr_type)
		error(0);
	    if (!expr_type)
		*dptr = k;		/* numeric assignment */
	    else {				/* character assignment */
		if (*dptr)
		    free((void *)dptr);		/* was free(*dptr); */
		if (*sa1) {
		    *dptr = (unsigned)allocate(strlen(sa1))+1;
		    strcpy((char *)*dptr, sa1);
		} else *dptr = 0;
	    }
	    break;

	case EXIT:
	    /* XXX Could there be any open files at this point? */
	    reclaim();
	    exit();

	case LIST:
	    chk_file(1);
	    if (!isdigit(skip_blank())) {
		i=0;
		j=-1;
	    } else {
		i = get_dnum();
		if (get_next() == ',') {
		    if (isdigit(skip_blank())) j=get_dnum();
		    else j = -1;
		} else j=i;
	    }
	    disp_pgm((uint32)fileout, i, j);
	    break;

	case NEW:
	    clear_vars();
	    clear_pgm();
	    longjmp(savjmp, 1);

	case RUN:
	    if (mode)
		error(1);		/* if not in direct mode */
	    clear_vars();		/* initialize all variables */

	case RUN1:			/* run w/o clearing variables */
	    if (is_e_end(skip_blank()))
		runptr = pgm_start;
	    else
		runptr = find_line(eval_num());
	    --mode;		/* indicate we're running */
newline:
	    while (runptr) {
		cmdptr = runptr->Ltext;
		line = runptr->Lnumber;
		do {
		    if ((cmd = skip_blank()) < 0) {
			++cmdptr;
			dptr = (unsigned *)execute(cmd);
			if (dptr) {
			    runptr = (struct line_record *)dptr;
			    goto newline;
			}
		    } else execute(1);
		} while((c = get_next()) == ':');
		if (c)
		    error(0);
		runptr = runptr->Llink;
	    }
	    mode = 0;
	    break;

	case CLEAR:
	    clear_vars();
	    break;

	case GOSUB:
	    ctl_stk[ctl_ptr++] = (unsigned)runptr;
	    ctl_stk[ctl_ptr++] = (unsigned)cmdptr;
	    ctl_stk[ctl_ptr++] = _GOSUB;

	case GOTO:
	    pgm_only();
	    return find_line(eval_num());

	case RETURN:
	    pgm_only();
	    if(ctl_stk[--ctl_ptr] != _GOSUB) error(6);
	    cmdptr = (char *)ctl_stk[--ctl_ptr];
	    runptr = (struct line_record *)ctl_stk[--ctl_ptr];
	    line = runptr->Lnumber;
	    skip_stmt();
	    break;

	case PRINT:
	    chk_file(1);

	    j = 0;
	    do {
		if (is_l_end(skip_blank())) {
		    j--;
		} else {
		    i = eval();
		    if (!expr_type) {
			num_string(i, sa1);
			fputc(' ',fileout);
		    }
		    fputs(sa1, fileout);
		}
	    } while (test_next(','));

	    if (!j)
		fputc('\n', fileout);
	    break;

	case FOR:
	    pgm_only();
	    ii = 1;			/* default step value */
	    i = get_var();
	    if (expr_type)		/* can't use a string var here */
		error(0);

	    expect(-128+EQ);		/* must have an equal sign */

	    num_vars[i] = eval();	/* evaluate the "from" expression */

	    if (expr_type)		/* it still can't be a string */
		error(0);

	    expect(-128+TO);		/* must have keyword "to" here */

	    jj = eval();		/* evaluate "to" expression */
					/* XXX why no test of expr_type? */

	    if (test_next(-128+STEP)) {	/* is there a "step" keyword? */
		ii = eval();		/* if so, evaluate te step size */
					/* XXX again, why no expr_type test */
	    }

	    skip_stmt();		/* advance cmdptr to next stmt */

	    /*-----------------------------*/
	    /* Setup the "for" loop frame. */
	    /*-----------------------------*/
	    ctl_stk[ctl_ptr++] = (unsigned)runptr;	/* line */
	    ctl_stk[ctl_ptr++] = (unsigned)cmdptr;	/* command pointer */
	    ctl_stk[ctl_ptr++] = ii;			/* step value */
	    ctl_stk[ctl_ptr++] = jj;			/* limit value */
	    ctl_stk[ctl_ptr++] = i;			/* variable index */
	    ctl_stk[ctl_ptr++] = _FOR;			/* type of "frame" */
	    break;

	case NEXT:
	    pgm_only();

	    if (ctl_stk[ctl_ptr-1] != _FOR)	/* we must be in a for loop */
		error(6);

	    i = ctl_stk[ctl_ptr-2];		/* get index of control var */

	    if (!is_l_end(skip_blank()))	/* did NEXT stmt give a var? */
	    	if (get_var() != i)		/* did it match as expected? */
		    error(6);			/* if not, we've got an err */

	    jj = ctl_stk[ctl_ptr-3];		/* get limit */
	    ii = ctl_stk[ctl_ptr-4];		/* get step */
	    num_vars[i] += ii;			/* increment the control var */

	    if ((ii < 0) ? num_vars[i] >= jj : num_vars[i] <= jj) {
		cmdptr = (char *)ctl_stk[ctl_ptr-5];
		runptr = (struct line_record *)ctl_stk[ctl_ptr-6];
		line = runptr->Lnumber;
	    } else {
		ctl_ptr -= 6;			/* for loop is done... */
	    }
	    break;

	case IF:
	    i = eval_num();
	    expect(-128+THEN);
	    if (i) {
		cmd = skip_blank();
		if (isdigit(cmd))
		    return find_line(eval_num());
		else if (cmd < 0) {
		    ++cmdptr;
		    return execute(cmd);
		} else execute(1);
	    } else skip_stmt();
	    break;

	case LIF:
	    i = eval_num();
	    expect(-128+THEN);
	    if (i) {
		if ((cmd = skip_blank()) < 0) {
		    ++cmdptr;
		    return execute(cmd);
		} else execute(1);
		break;
	    }
/* This looks strange, but that's the way it is in the last version of */
/* the code I examined. I may need to go back further. If the 'if (i)' */
/* is false, then this drops into the next case!                       */

	case DATA:
	    pgm_only();

	case REM :
	    if (mode) {
		cptr = runptr->Llink;
		if (cptr)
		    return cptr;
		longjmp(savjmp, 1);
	    }
	    break;

	case STOP:
	    pgm_only();
	    printf("STOP in line %u\n", line);

	case END:
	    pgm_only();
	    longjmp(savjmp, 1);

	case INPUT:
	    ii = chk_file(1);
	    if (skip_blank() == '"') {		/* special prompt */
		eval();
		expect(',');
	    } else strcpy(sa1, "? ");
	    dptr = address();
	    cptr = (struct line_record *)cmdptr;

input:	    if (ii == -1)			/* prompt for input? */
		fputs(sa1, stdout);

	    cmdptr = fgets(buffer, sizeof(buffer)-1 ,filein);

	    if (expr_type) {
		if (*dptr) {
		    free((void *)*dptr);	/* free existing allocation */
		}
		if (cmdptr == NULL) {		/* end of file? */
		    *dptr = (unsigned)allocate(1);
		    **(char **)dptr = '\0';    /* return empty string */
		} else {
		    *dptr = (unsigned)allocate(strlen(buffer)+1);
		    strcpy((char *)*dptr, buffer); 
		}
	    } else {
		/* XXX What do we do with EOF on numeric input? */
		k = 0;
		if (test_next('-')) --k;
		if (!isdigit(*cmdptr)) {
		    if (ii != -1) error(9);
		    fputs("Input error\n",stdout);
		    goto input;
		}
		j = get_num();
		*dptr = (k) ? 0-j: j;
	    }
	    cmdptr = (char *)cptr;
	    break;

	case OPEN:
	    if (skip_blank() != '#')
		error(0);
	    if (files[i = chk_file(0)])
		error(8);
	    eval_char();
	    strcpy(buffer, sa1);		/* save file name */
	    expect(',');
	    eval_char();			/* sa1 = mode string */
	    syscr = open(NAMESPACE, buffer, sa1);	/* try open */
	    if (syscr == SYSERR)
		error(13);				/* maybe wrong error */
	    files[i] = syscr;
	    break;

	case CLOSE:
	    if ((i = chk_file(1)) == -1)
		error(0);
	    if (!filein)
		error(8);
	    syscr = close(filein);
	    if (syscr == SYSERR)
		error(7);
	    files[i] = 0;
	    break;

	case DIM:
	    do {
		int *temp;

		i = get_var();
		temp = dim_vars[i];
		if (temp)
		    free((void *)temp);
		/* was: if (dptr = dim_vars[i = get_var()]) free(dptr); */

		
		dim_check[i] = eval_num() + 1;
		if (dim_check[i] < 1)
		    error(10);
		dim_vars[i] = (int *)allocate(dim_check[i] * sizeof(int));
		/* was: dim_vars[i] =
		    (int *)allocate((dim_check[i] = eval_num()+1) * 2); */

	    } while(test_next(','));
	    break;

	case ORDER:
	    readptr = find_line(eval_num());
	    dptr = (unsigned *)cmdptr;
	    cmdptr = readptr->Ltext;
	    if (get_next() != -128+DATA) error(11);
	    dataptr = cmdptr;
	    cmdptr = (char *)dptr;
	    break;

	case READ:
	    do {
		dptr = address();
		j = expr_type;
		cptr = (struct line_record *)cmdptr;
		cmdptr = dataptr;
		ii = line;
		if (!skip_blank()) {		/* End of line */
		    readptr = readptr->Llink;
		    cmdptr = readptr->Ltext;
		    if (get_next() != -128+DATA) error(11);
		}
		line = readptr->Lnumber;
		k = eval();
		test_next(',');
		dataptr = cmdptr;
		cmdptr = (char *)cptr;
		line = ii;
		if (j != expr_type) error(11);
		if (!expr_type) *dptr = k;	/* numeric assignment */
		else {				/* character assignment */
		    if (*dptr)
			free((void *)dptr);
		    if (*sa1) {
			*dptr = (unsigned)allocate(strlen(sa1)+1);
			strcpy((char *)*dptr, sa1);
		    } else *dptr = 0;
		}
	    } while(test_next(','));
	    break;

	case XDELAY :
	    delay(eval_num());
	    break;

	case OUT1:
	    port = eval_num();		/* get the port number */
	    if (port < 0 || port > 65535)
		error(14);
	    expect(',');
	    j = eval_num();		/* get the value to store */
	    if (!is_l_end(skip_blank()))
		error(0);		/* trailing garbage in stmt */
	    outb(port,j);
	    break;

	case OUT2:
	    port = eval_num();		/* get the port number */
	    if (port < 0 || port > 65535)
		error(14);
	    expect(',');
	    j = eval_num();		/* get the value to store */
	    if (!is_l_end(skip_blank()))
		error(0);		/* trailing garbage in stmt */
	    outb(port,j);
	    break;

	case OUT4:
	    port = eval_num();		/* get the port number */
	    if (port < 0 || port > 65535)
		error(14);
	    expect(',');
	    j = eval_num();		/* get the value to store */
	    if (!is_l_end(skip_blank()))
		error(0);		/* trailing garbage in stmt */
	    outb(port,j);
	    break;
#if 0
	    /*--------------------------------------------------------*/
	    /* THIS IS THE OLD CODE FOR "OUT" WHICH STORED THE SECOND */
	    /* VALUE AT THE ADDRESS GIVEN BY THE FIRST VALUE.         */
	    /* WE WILL RENAME THIS LATER.                             */
	    /*--------------------------------------------------------*/
	    i = eval_num();		/* evaluate address */
					/* should we check range? */
	    expect(',');

	    j = eval_num();		/* evalute value to store there */
	    if (j > 255)
		error(4);		/* treat as wrong type, for now */

	    *(char *)i = (char)j;	/* store it */

	    break;
#endif

	case SAVE:
	    if (mode)			/* verify we're in direct mode */
		error(1);
	    if (skip_blank()) {
		eval_char();
		strcpy(filename,sa1);
	    }
	    fileout = open(NAMESPACE, filename, "w");	/* try open */

	    if (fileout != (did32)SYSERR) {		/* if successful */
		disp_pgm(fileout, 0, -1);
		close(fileout);
	    } else
		error(13);
	    break;

	case LOAD:
	    eval_char();
	    strcpy(filename,sa1);
	    filein = open(NAMESPACE, filename, "ro");	/* try open */

	    if (filein == (did32)SYSERR) {		/* no success */
		error(13);
		longjmp(savjmp, 1);
	    }

	    if(!mode)					/* if running */
		clear_vars();

	    clear_pgm();

	    while(fgets(buffer, sizeof(buffer)-1, filein))
		edit_program();
	    close(filein);

	    return pgm_start;

	default :			/* unknown */
	    error(0);
    }
    return 0;
}

/*-------------------------------------------------------------------*/
/* Test for file operator, and set up pointers                       */
/*                                                                   */
/* That is, test to see if the next item (e.g. after INPUT or PRINT) */
/* is #num, where num is a file number. If so, set the global vars   */
/* 'filein' and 'fileout' to the appropriate stream. If an input     */
/* operation is underway and the file isn't open and flag is nonzero */
/* then report error 7 (file not open). If the file number is too    */
/* large (for input or output), also report error 7.                 */
/*-------------------------------------------------------------------*/
static uint32 chk_file(char flag)
{
    uint32 i;

    i = -1;
    if (test_next('#')) {		/* file number specified? */
	i = eval_num();			/* get file number */
	if (i > MAXFILES-1)
	    error(7);

	test_next(',');
	filein = fileout = files[i];
	if (flag && (!filein))
	    error(7);
    } else {
	filein = stdin;
	fileout = stdout;
    }
    return i;
}

/*----------------------------------*/
/* Display lines i through j of the */
/* program on the specified stream. */
/*----------------------------------*/
static void disp_pgm(uint32 fp, uint32 i, uint32 j)
{
    uint32 k;
    struct line_record *cptr;
    char c;

    for (cptr = pgm_start; cptr != NULL; cptr = cptr->Llink) {
	k = cptr->Lnumber;
	if ((k >= i) && (k <= j)) {
	    fprintf(fp,"%u ", k);
	    for (k=0; (c = cptr->Ltext[k]); ++k) {
		if(c < 0) {
		    c = c & 0x7f;
		    fputs(reserved_words[c - 1], fp);
		    if(c < ADD) fputc(' ',fp);
		} else fputc(c,fp);
	    }
	    fputc('\n', fp);
	}
    }
}

/*--------------------------------------------*/
/* Test to see if we're in program only mode. */
/* If in interactive mode, generate an error. */
/*--------------------------------------------*/
static void pgm_only(void)
{
    if (!mode)
	error(2);
}

/*----------------------------------------------------------*/
/* Advance cmdptr through the end of the current statement. */
/*----------------------------------------------------------*/
static void skip_stmt(void)
{
    char c;

    while ((c = *cmdptr) && (c != ':')) {
	cmdptr++;
	if(c == '"') {
	    while ((c = *cmdptr) && (c != '"'))
		cmdptr++;
	    if (c)
		cmdptr++;
	}
    }
}

/*------------------------------------------------*/
/* Display the error message for error number en. */
/*------------------------------------------------*/
static void error(uint32 en)
{
    printf("%s", error_messages[en]);
    if (mode)
	printf(" in line %u", line);
    fputc('\n',stdout);
    longjmp(savjmp, 1);
}

/*---------------------------------------------*/
/* Evaluate a numeric expression. Generate an  */
/* error if the result is not a numeric value. */
/* The expression's value is left in 'value'.  */
/*---------------------------------------------*/
static int32 eval_num(void)
{
    int32 value;

    value = eval();
    if (expr_type)		/* if it's not numeric... */
	error(4);
    return value;
}

/*-----------------------------------------------*/
/* Evaluate a character expression. Generate an  */
/* error if the result is not a character value. */
/*-----------------------------------------------*/
static void eval_char(void)
{
    (void)eval();
    if (!expr_type)
	error(4);
}

/*------------------------------------------------------*/
/* Evaluate an expression (numeric or character).       */
/* A numeric value is returned as the function's value. */
/* A string value is returned in sa1.                   */
/*------------------------------------------------------*/
static int32 eval(void)
{
    int32 value;

    nest = 0;
    value = eval_sub();
    if (nest != 1)
	error(0);
    return value;
}

/*---------------------------*/
/* Evaluate a subexpression. */
/*---------------------------*/
static int32 eval_sub(void)
{
    /* The next line previously declared things as "unsigned" */
    int32 value, nstack[10], nptr, optr;
    char c, ostack[10];

    ++nest;		/* indicate we went down */

    /*-----------------------------------------------------*/
    /* establish first entry on number and operator stacks */
    /*-----------------------------------------------------*/
    nptr = 0;				/* value stack is empty */
    optr = 0;				/* operator stack is empty */
    ostack[optr] = 0;			/* add zero to init */

    nstack[++nptr] = get_value();	/* get next value */

    /*-------------------*/
    /* string operations */
    /*-------------------*/
    if (expr_type) {
	while (!is_e_end(c = skip_blank())) {
	    ++cmdptr;
	    c &= 0x7F;
	    get_char_value(sa2);
	    if(c == ADD)		/* concatenation */
		strcat(sa1, sa2);
	    else {
		if (c == EQ)
		    value = !strcmp(sa1, sa2);
		else if(c == NE)
		    value = (strcmp(sa1, sa2) != 0);
		else
		    error(0);
		nstack[nptr] = value;
		expr_type = 0;
	    }
	}

    } else {

    /*--------------------*/
    /* numeric operations */
    /*--------------------*/
	c = skip_blank();
	while (!is_e_end(c)) {
	    ++cmdptr;
	    c = (c & 0x7F) - (ADD-1);	/* 0 based priority table */

	    /*--------------------------------------------------*/
	    /* Does top of stack operator have higher priority? */
	    /*--------------------------------------------------*/
	    if (priority[(int)c] <= priority[(int)ostack[optr]]) {

		/*----------------------------------*/
		/* Yes, so apply that operator now. */
		/*----------------------------------*/
		value = nstack[nptr--];
		nstack[nptr] = do_arith(ostack[optr--], nstack[nptr], value);
	    }
	    nstack[++nptr] = get_value();		/* stack next operand */
	    if (expr_type) error(0);
	    ostack[++optr] = c;
	    c = skip_blank();
	}

	while(optr) {			/* do all reaining operations */
	    value = nstack[nptr--];
	    nstack[nptr] = do_arith(ostack[optr--], nstack[nptr], value);
	}
    }

    if (c == ')') {
	--nest;
	++cmdptr;
    }

    return nstack[nptr];
}

/*****************************************/
/* Get a value element for an expression */
/*****************************************/
static int32 get_value(void)
{
    int32 value, v, *dptr;
    int32 port;				/* port number for in1, in2, in4 */
    char c, *ptr;

    value = 0;				/* appease the compiler */
    port = 0;
    expr_type = 0;
    c = skip_blank();
    if (isdigit(c))
	value = get_num();
    else {
	cmdptr++;
	switch(c) {

	    case '(':			/* nesting */
		value = eval_sub();
		break;

	    /* XXX Is the tilde operator really what we want? */
	    /* XXX Or should we instead use ! ? */
	    case '!':			/* not */
		value = ~get_value();
		break;

	    case -128+SUB:		/* negate */
		value = -get_value();
		break;

	    case -128+ASC:		/* Convert character to number */
		eval_sub();
		if (!expr_type)		/* Here we require a string */
		    error(4);
		value = *sa1 & 255;
		expr_type = 0;
		break;

	    case -128+NUM:		/* Convert string to number */
		eval_sub();
		if (!expr_type)		/* String arg is required */
		    error(4);
		value = atoi(sa1);	/* convert string to binary */
		expr_type = 0;		/* indicate result type */
		break;

	    case -128+ABS:		/* Absolute value */
		value = eval_sub();
		if (value < 0)
		    value = -value;	/* fails on 0x80000000 */
		goto number_only;

	    case -128+RND:		/* Random number */
		value = random(eval_sub());
		goto number_only;

	    case -128+STRLEN:		/* string length */
		eval_sub();
		if (!expr_type)		/* String arg is required */
		    error(4);
		value = strlen(sa1);	/* XXX Unsure if this is adequate */
		expr_type = 0;		/* indicate result type */
		break;

	    case -128+MEM:		/* Read from memory */
		value = rdmem(eval_sub());

	    number_only:
		if (expr_type)		/* If not numeric... */
		    error(4);
		break;

	    case -128+IN1:		/* Read one byte from a port */
		port = eval_sub();
		if (expr_type != 0)
		    error(14);
		if (port < 0 || port > 65535)
		    error(14);
		value = inb(port);
		expr_type = 0;
		break;
	    case -128+IN2:		/* Read two bytes from a port */
		port = eval_sub();
		if (expr_type != 0)
		    error(14);
		if (port < 0 || port > 65535)
		    error(14);
		value = inw(port);
		expr_type = 0;
		break;
	    case -128+IN4:		/* Read four bytes from a port */
		port = eval_sub();
		if (expr_type != 0)
		    error(14);
		if (port < 0 || port > 65535)
		    error(14);
		value = inl(port);
		expr_type = 0;
		break;

	    default:			/* test for character expression */
		cmdptr--;
		if (isalpha(c)) {	/* variable */
		    value = get_var();
		    if (expr_type) {	/* char */
			ptr = char_vars[value];
			if (ptr)
			    strcpy(sa1, ptr);
			else
			    strcpy(sa1, "");
		    } else {
			if (test_next('(')) {	/* Array */
			    dptr = dim_vars[value];
			    if (!dptr)		/* verify array was DIM'ed */
				error(10);
			    v = eval_sub();
			    if (v >= dim_check[value])	/* verify subscript */
				error(10);
			    if (v < 0)
				error(10);
			    value = dptr[v];	/* get array element's value */
			} else 
			    value = num_vars[value]; /* get scalar's value */
		    }
		} else
		    get_char_value(sa1);	/* get string's value */
	}
    }
    return value;
}

/***********************/
/* Get character value */
/***********************/
static void get_char_value(char *ptr)
{
    unsigned i;
    char c, *st;

    if ((c = get_next()) == '"') {	/* character value */
	while ((c = *cmdptr++) != '"') {
	    if(!c) error(0);
	    *ptr++ = c;
	}
	*ptr = 0;
    } else if (isalpha(c)) {			/* variable */
	--cmdptr;
	i = get_var();
	if (!expr_type)
	    error(0);
	st = char_vars[i];
	if (st)
	    strcpy(ptr,st);
	else
	    strcpy(ptr,"");
    } else if (c == -128+CHR) {		/* Convert number to character */
	*ptr++ = eval_sub();
	if (expr_type)
	    error(4);
	*ptr = 0;
    } else if (c == -128+STR) {		/* Convert number to string */
	num_string(eval_sub(), ptr);
	if (expr_type)
	    error(4);
    } else if (c == -128+HEX) {		/* Convert number to hex string */
	hex_string(eval_sub(), ptr);
	if (expr_type)
	    error(4);
    } else
	error(0);
    expr_type = 1;
}

/***********************************/
/* Perform an arithmetic operation */
/***********************************/
static int do_arith(char opr, unsigned op1, unsigned op2)
{
    unsigned value;

    switch(opr) {

	case ADD-(ADD-1):		/* addition */
	    value = op1 + op2;
	    break;

	case SUB-(ADD-1):		/* subtraction */
	    value = op1 - op2;
	    break;

	case MUL-(ADD-1):		/* multiplication */
	    value = op1 * op2;
	    break;

	case DIV-(ADD-1):		/* division */
	    value = op1 / op2;
	    break;

	case MOD-(ADD-1):		/* modulus */
	    value = op1 % op2;
	    break;

	case AND-(ADD-1):		/* logical and */
	    value = op1 & op2;
	    break;

	case OR-(ADD-1):		/* logical or */
	    value = op1 | op2;
	    break;

	case XOR-(ADD-1):		/* exclusive or */
	    value = op1 ^ op2;
	    break;

	case EQ-(ADD-1):		/* equals */
	    value = op1 == op2;
	    break;

	case NE-(ADD-1):		/* not-equals */
	    value = op1 != op2;
	    break;

	case LE-(ADD-1):		/* less than or equal to */
	    value = op1 <= op2;
	    break;

	case LT-(ADD-1):		/* less than */
	    value = op1 < op2;
	    break;

	case GE-(ADD-1):		/* greater than or equal to */
	    value = op1 >= op2;
	    break;

	case GT-(ADD-1):		/* greater than */
	    value = op1 > op2;
	    break;

	default:
	    error(0);
    }
    return value;
}

/*----------------------------------------------------------------*/
/* Convert a 32-bit binary integer 'value' to a string of decimal */
/* digits in memory starting at 'ptr'. Only include a sign if     */
/* the number is negative.                                        */
/* Note that there is no check for the string overflowing the     */
/* storage area pointed to by 'ptr'. It should be at least 12     */
/* bytes long to handle the largest negative value and the null   */
/* byte always added at the end of the string.                    */
/*----------------------------------------------------------------*/
static void num_string(unsigned value, char *ptr)
{
    char cstack[10];
    int32 cptr;

    if (value == 0x80000000) {		/* special case */
	strcpy(ptr,"-2147483648");
	return;
    }

    cptr = 0;
    if (value > 0x7fffffff) {		/* is it negative? */
	*ptr++ = '-';			/* yes, so stash a minus sign */
	value = -value;			/* and negate the value */
    }

    do {				/* convert number to decimal digits */
	cstack[cptr++] = (value % 10) + '0';
	value /= 10;
    } while(value);

    while (cptr)			/* copy digits in reverse order */
	*ptr++ = cstack[--cptr];
    *ptr = 0;
}

/*----------------------------------------------------------------*/
/* Convert a 32-bit binary integer 'value' to an 8-byte string of */
/* hexadecimal digits starting at 'ptr'.                          */
/*----------------------------------------------------------------*/
static void hex_string(unsigned value, char *ptr)
{
    int32 i, dv;

    for(i=0;i<8;i++) {
	dv = value & 0x0f;
	if (dv <= 9)
	    ptr[7-i] = '0' + dv;
	else
	    ptr[7-i] = 'a' + dv - 10;
	value = value >> 4;
    }
    ptr[8] = '\0';
}
    

/****************************/
/* Clear program completely */
/****************************/
static void clear_pgm(void)
{
    for (runptr = pgm_start; runptr; runptr = runptr->Llink) free(runptr);
    pgm_start = 0;
}

/*******************************/
/* Clear all variables to zero */
/*******************************/
static void clear_vars(void )
{
    unsigned i;
    char *ptr;

    for (i=0; i < NUM_VAR; ++i) {
	num_vars[i] = 0;
	ptr = char_vars[i];
	if (ptr) {				/* Character variables */
	    free((void *)ptr);
	    char_vars[i] = 0;
	}
	ptr = (char *)dim_vars[i];
	if (ptr) {				/* Dimensioned arrays */
	    free((void *)ptr);
	    dim_vars[i] = 0;
	}
    }
}

/*-----------------------------------------------------------*/
/* Examine the next item in the statement, which should be a */
/* variable  name. If it isn't, generate a syntax error.     */
/* Then generate the 'index' for the variable name. This is  */
/* 10 times the letter -'A' value, plus the digit's value,   */
/* if any. So A0 has the index 0, A9 has the index 9, B0 has */
/* the index 10, and Z9 has the index 259.                   */
/*							     */
/* Set the expr_type to 1 if the next character is '$'.      */
/* That is, the variable is a string variable.		     */
/* Otherwise set the expr_type to 0 (integer).               */
/*							     */
/* In all cases, leave cmdptr pointing to the first unused   */
/* character in the statement.                               */
/*-----------------------------------------------------------*/
static uint32 get_var(void)
{
    uint32 index;
    char c;

    c = get_next();
    if (!isalpha(c))
	error(0);
    index = ((c - 'A') & 0x1F) * 10;

    c = *cmdptr;
    if (isdigit(c)) {
	index += (c - '0');
	c = *++cmdptr;
    }

    if (c == '$') {
	++cmdptr;
	expr_type = 1;		/* character */
    } else
	expr_type = 0;		/* integer */

    return index;
}

shellcmd xsh_basic(int argc, char *argv[])
{
    int32 i, j, k, len;

    prog_mode = 1;			/* assume interactive */
    init_vars();			/* guarantee variable initialization */
    mode = 0;				/* other initializations */
    ctl_ptr = 0;

#if 0
    xp2 = xp + 1;
    printf("&xp = %d, value = %d\n", (int)&xp, (int)xp);
    printf("&xp2 = %d, value = %d\n", (int)&xp2, (int)xp2);
    printf("&xp3 = %d, value = %d\n", (int)&xp3, (int)xp3);
    printf("&xsh_basic = %d, value = %d\n",
	(int)xsh_basic, (int)*(unsigned char *)xsh_basic);
#endif

    /*----------------------------------------------------------*/
    /* If arguments are given, copy them into A0$, A1$, A2$ ... */
    /*----------------------------------------------------------*/
    pgm_start = NULL;
    j = 0;
    for(i=1; i < argc; ++i)
	char_vars[j++] = strdup(argv[i]);

    /*-------------------------------------------------------------------*/
    /* If a name was given on the command line, see if it is the name of */
    /* a BASIC program. That is, if argv[1] or argv[1] suffixed with     */
    /* ".bas" is a file that can be opened, then assume it's a program.  */
    /* In these cases, we execute the program in the "non-interactive"   */
    /* mode. that is, we don't display the interpreter version string,   */
    /* and when the program is finished, we don't display a stop message */
    /* but just exit the interpreter.                                    */
    /*-------------------------------------------------------------------*/
    /* N.B. For now, we'll not attempt to modify the case of anything on */
    /* the command line. That is, the user MUST make certain the file    */
    /* name ends in lowercase ".bas" if it is to be executed.            */
    /*-------------------------------------------------------------------*/
    if (j) {		/* was there at least one command-line argument? */
	strcpy(filename, char_vars[0]);

	filein = open(NAMESPACE, filename, "ro");	/* try open */
	if (filein == (did32)SYSERR) {
	    concat(filename, char_vars[0], ".bas");
	    filein = open(NAMESPACE, filename, "ro");	/* try open w/ .bas */
	}

	/*--------------------------------------------*/
	/* If open was successful, read each line and */
	/* send it to 'edit_program'.                 */
	/*--------------------------------------------*/
	if (filein != (did32)SYSERR) {
	    while (fgets(buffer, sizeof(buffer)-1, filein) != NULL)
		edit_program();
	    close(filein);

	    if (!setjmp(savjmp)) {
		execute(RUN1);
		/* XXX Should we check for open files here? */
		reclaim();
		exit();
	    }
	}
    }

    fputs(version,stdout);	/* XXX do this only if in interactive mode */

    setjmp(savjmp);
    for(;;) {				/* Main interactive loop */
	fputs("Ready\n", stdout);

noprompt:
	mode = 0;
	ctl_ptr = 0;
	if (fgets(buffer, sizeof(buffer)-1, stdin) == NULL) {
	    printf("Terminating on end of file.\n");
	    reclaim();
	    exit();
	}

	/*----------------------------------------------------------------*/
	/* Verify a complete line was read. There must be an end of line  */
	/* character present. Otherwise, report an error, ignore the rest */
	/* of the line (or until end of file), then continue the loop.    */
	/*----------------------------------------------------------------*/
	len = strlen(buffer);
	if (len == 0)				/* pathological */
	    continue;
	if (buffer[len-1] != '\n') {		/* line too long */
	    printf("Too long.\n");
	    char *fgr;				/* fgets result */
	    while ((fgr = fgets(buffer, sizeof(buffer)-1, stdin)) != NULL) {
		len = strlen(buffer);
		if (len == 0)
		    continue;
		if (buffer[len-1] != '\n')	/* really long! */
		    continue;
	    }
	    if (fgr == NULL) {
		reclaim();
		exit();
	    }
	    continue;
	}

	/*------------------------------------*/
	/* identify tokens in the input line. */
	/*------------------------------------*/
	k = edit_program();

	/*---------------------------------------*/
	/* If the line had a statement number... */
	/*---------------------------------------*/
	if (k)
	    goto noprompt;

	/*-------------------------------*/
	/* Execute an immediate command. */
	/*-------------------------------*/
	i = *cmdptr;
	if (i < 0) {			/* Keyword, execute command */
	    ++cmdptr;
	    execute(i);
	} else if (i)			/* assume an immediate LET statement */
	    execute(LET);
    }
}
