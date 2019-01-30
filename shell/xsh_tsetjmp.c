/* xsh_tsetjmp.c - xsh_tsetjmp */

#include <xinu.h>
#include <stdio.h>
#include <string.h>

static jmp_buf emain1;			/* set by first setjmp in main */
static jmp_buf emain2;			/* set by second setjmp in main */
static jmp_buf ef1;			/* set by setjmp in f1 */
static jmp_buf ef2;			/* set by setjmp in f1 */

/*------------------------------------------------------------------------
 * xsh_tsetjmp - demonstrate the setjmp/longjmp functions
 *------------------------------------------------------------------------
 */
int getans(void)
{
	int32 c;

	for(;;) {
	    c = getc(stdin);
	    if (c == EOF) {
		printf("Terminating on end of file.\n");
		exit();
	    }
	    if (c == 'y' || c == 'Y') {
#if 0
		printf("Returning 1 from getans.\n");
#endif
		return 1;
	    }
	    if (c == 'n' || c == 'N') {
#if 0
		printf("Returning 0 from getans.\n");
#endif
		return 0;
	    }
#if 0
	    printf("Ignoring input character 0x%08x\n", c);
#endif
	}
}

static void f3(void)
{
	printf("Executing inside function f3.\n");
	printf("Do you want to do a longjmp to main point 1?\n");
	if (getans())
	    longjmp(emain1, 1);
	printf("Do you want to do a longjmp to main point 2?\n");
	if (getans())
	    longjmp(emain2, 2);
	printf("Do you want to do a longjmp to f1?\n");
	if (getans())
	    longjmp(ef1, 7);
	printf("Do you want to do a longjmp to f2?\n");
	if (getans())
	    longjmp(ef2, 8);
	printf("Normally returning from f3.\n");
}

static void f2(void)
{
	int32 k;

	printf("Executing inside function f2.\n");
	k = setjmp(ef2);
	if (k != 0)
	    printf("--->Jumped back to f2 from f3; value = %d.\n", k);
	printf("Invoking f3...\n");
	f3();
	printf("Do you want to do a longjmp to main point 1?\n");
	if (getans())
	    longjmp(emain1, 3);
	printf("Do you want to do a longjmp to main point 2?\n");
	if (getans())
	    longjmp(emain2, 4);
	printf("Normally returning from f2.\n");
}

static void f1(void)
{
	int32 k;

	printf("Executing inside function f1.\n");
	k = setjmp(ef1);
	if (k != 0)
	    printf("--->Jumped back to f1 from f2; value = %d.\n", k);
	printf("Invoking f2...\n");
	f2();
	printf("Do you want to do a longjmp to main point 1?\n");
	if (getans())
	    longjmp(emain1, 5);
	printf("Do you want to do a longjmp to main point 2?\n");
	if (getans())
	    longjmp(emain2, 6);
	printf("Normally returning from f1.\n");
}
	
shellcmd xsh_tsetjmp(int nargs, char *args[])
{
	int32	k;

	/* For argument '--help', emit help about the 'tsetjmp' command	*/

	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
		printf("Use: %s [--help]\n\n", args[0]);
		printf("Description:\n");
		printf("This program demonstrates the operation of the\n");
		printf("setjmp and longjmp functions. The functions are\n");
		printf("defined exactly as they are on many other systems.\n");
		printf("\n");
		printf("At various points you'll be asked if you want to do ");
		printf("a longjmp.\n");
		printf("Just answer y or Y for yes, and n or N for no.\n");
		printf("End of file will terminate the program early.\n");
		printf("You can also quit by repeatedly typing n or N.\n");
		return 0;
	}

	if (nargs != 1) {
		fprintf(stderr,"Wrong invocation; type %s --help for "
		    "usage information.\n", args[0]);
		return 0;
	}

	k = setjmp(emain1);
	if (k != 0)
	    printf("Returned to main point 1; value = %d\n", k);
	k = setjmp(emain2);
	if (k != 0)
	    printf("Returned to main point 2; value = %d\n", k);
	printf("Invoking f1...\n");
	f1();
	printf("main function is terminating.\n");
	return 0;
}
