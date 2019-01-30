/* lfscreate.c - lfscreate */

/* 10/16/2016 - Set the in memory and dirty flags to false. */

#include <xinu.h>
#include <ramdisk.h>

struct lfdata Lf_data;

/*------------------------------------------------------------------------
 * lfscreate  -  Create an initially-empty file system on a disk
 *------------------------------------------------------------------------
 */
status	lfscreate (
	  did32		disk,		/* ID of an open disk device	*/
	  ibid32	lfiblks,	/* Num. of index blocks on disk	*/
	  uint32	dsiz		/* Total size of disk in bytes	*/
	)
{
	uint32	sectors;		/* Number of sectors to use	*/
	uint32	ibsectors;		/* Number of sectors of i-blocks*/
	uint32	ibpersector;		/* Number of i-blocks per sector*/
	struct	lfdir	dir;		/* Buffer to hold the directory	*/
	uint32	dblks;			/* Total free data blocks	*/
	struct	lfiblk	iblock;		/* Space for one i-block	*/
	struct	lfdbfree dblock;	/* Data block on the free list	*/
	dbid32	dbindex;		/* Index for data blocks	*/
	int32	retval;			/* Return value from func call	*/
	int32	i;			/* Loop index			*/

	/* Compute total sectors on disk */

	sectors = dsiz	/ LF_BLKSIZ;	/* Truncate to full sector */

	/* Compute number of sectors comprising i-blocks */

	ibpersector = LF_BLKSIZ / sizeof(struct lfiblk);
	ibsectors = (lfiblks+(ibpersector-1)) / ibpersector;/* Round up	*/
	lfiblks = ibsectors * ibpersector;
	if (ibsectors > sectors/2) {	/* Invalid arguments */
		return SYSERR;
	}

	/* Create an initial directory */

	memset((char *)&dir, NULLCH, sizeof(struct lfdir));
	dir.lfd_nfiles = 0;
	dir.lfd_magic[0] = 'L';
	dir.lfd_magic[1] = 'F';
	dir.lfd_magic[2] = 'S';
	dir.lfd_magic[3] = 'Y';
	dbindex= (dbid32)(ibsectors + 1);
	dir.lfd_dfree = dbindex;
	dblks = sectors - ibsectors - 1;
	retval = write(disk,(char *)&dir, LF_AREA_DIR);
	if (retval == SYSERR) {
		return SYSERR;
	}

	/* Create list of free i-blocks on disk */

	lfibclear(&iblock, 0);
	for (i=0; i<lfiblks-1; i++) {
		iblock.ib_next = (ibid32)(i + 1);
		lfibput(disk, i, &iblock);
	}
	iblock.ib_next = LF_INULL;
	lfibput(disk, i, &iblock);

	/* Create list of free data blocks on disk */

	memset((char*)&dblock, NULLCH, LF_BLKSIZ);
	for (i=0; i<dblks-1; i++) {
		dblock.lf_nextdb = dbindex + 1;
		write(disk, (char *)&dblock, dbindex);
		dbindex++;
	}
	dblock.lf_nextdb = LF_DNULL;
	write(disk, (char *)&dblock, dbindex);
	close(disk);

	Lf_data.lf_dirpresent = Lf_data.lf_dirdirty = FALSE;
	
	return OK;
}
