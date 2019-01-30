/* xsh_echo.c - xsh_echo */

#include <xinu.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * xhs_echo - write argument strings to stdout
 *------------------------------------------------------------------------
 */
shellcmd xsh_echo(int nargs, char *args[])
{
	int32	i;			/* walks through args array	*/

#if 0
	uint32	v;			/* value from getcr0, getcr3	*/

	v = getcr0();
	printf("cr0 = 0x%08x\n", v);

	v = getcr3();
	printf("cr3 = 0x%08x\n", v);
#endif

	if (nargs > 1) {
		printf("%s", args[1]);

		for (i = 2; i < nargs; i++) {
			printf(" %s", args[i]);
		}
	}
	printf("\n");

	return 0;
}
