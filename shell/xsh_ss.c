/* xsh_ss.c - xsh_ss */

#include <xinu.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * xsh_ss - Start a shell process using the UE0 device.
 *------------------------------------------------------------------------
 */
shellcmd xsh_ss(int nargs, char *args[])
{
    did32 descr;		/* file descriptor */

    if (nargs == 2 && strcmp(args[1],"--help")) {
	printf("Use: ss [--help]\n\n");
	printf("Description:\n");
	printf("    Start a second shell process (named sshell) that will\n");
	printf("    communicate with a system running the utel program.\n");
	printf("    Communication uses the UE0 device on the Xinu system.\n");
	printf("Options:\n");
	printf("    --help    display this help and exit\n");
	return 0;
    }

    if (nargs != 1) {
	printf("Bad invocation; try \"ss --help\" for more information.\n");
	return 0;
    }

    /*-------------------------------------------------------------*/
    /* Open the UE0 device. This will block until the utel program */
    /* is executed on the remote system and conacts this Xinu box. */
    /*-------------------------------------------------------------*/
    descr = open(UE0,"unused","unused");

    /*--------------------------------------------------------------*/
    /* Now start a shell that communicates using the descriptor for */
    /* the UE0 device, then return. The sshell process continues to */
    /* run until the remote utel program terminates.                */
    /*--------------------------------------------------------------*/
    resume(create(shell, 8192, 50, "sshell", 1, descr));
    return OK;
}
