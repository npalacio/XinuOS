
/*-------------------------------------------------------------------------
 * rename - Change the name of a file
 *-------------------------------------------------------------------------
 */

#include <xinu.h>

shellcmd xsh_rename(int nargs, char *args[])
{
    int status;

    if (nargs == 2 && strcmp(args[1],"--help") == 0) {
	printf("Usage:\n    rename  oldname  newname\n");
	return 0;
    }

    if (nargs != 3) {
	printf("Usage:\n    rename  oldname  newname\n");
	return 0;
    }

    status = rename(args[1],args[2]);
    if (status != OK) {
	printf("Cannot rename %s to %s\n", args[1], args[2]);
	printf("errno = %d\n", proctab[currpid].errno);
    }

    return 0;
}
