/* errors.h */

/*-------------------------------------------------------------*/
/* Definition of values for errno                              */
/* These are found in the errno member of the process table.   */
/* Some attempt has been made to use the same numbers as UNIX. */
/* The numbers on Linux systems are typically found in the     */
/* files /usr/include/asm-generic/errno-base.h or              */
/* /usr/include/asm-generic/errno.h . The easy way to find the */
/* value of something is find its symbolic name and then grep  */
/* for it (several layers may need to be examined).            */
/*-------------------------------------------------------------*/

#define	ENONE		0		/* no error */

#define ENOENT		2		/* no such file or directory */

#define EBADF		9		/* bad file number */

#define EAGAIN		11		/* try again (create, perhaps) */

#define EEXIST		17		/* file exists */

#define EXDEV		18		/* cross-device link */

#define EINVAL		22		/* invalid argument */

#define ENOSPC		28		/* no space left on device */

#define ERANGE		34		/* math result not representable */

#define ENAMETOOLONG	36		/* file name too long */

#define ENOSLAVE	200		/* no slave file devices available */
#define EISOPEN		201		/* file is already open */
#define EUNIMP		202		/* not yet implemented */
#define ENOARGVAL	203		/* argument value not found on stack */
#define EBADMAGIC	204		/* bad magic number (eg, in filesys) */
