/* lfscontrol.c - lfscontrol */

#include <xinu.h>

/*------------------------------------------------------------------------
 * lfscontrol  -  Provide control functions for a local filesystem master
 *------------------------------------------------------------------------
 */

/*--------------------------------------------------------------------*/
/* XXX								      */
/* We should probably write a few functions to check filename length  */
/* and make the directory present in memory if it isn't already. Also */
/* we need to be able to tell if a directory on disk "looks" okay.    */
/*--------------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/* Return TRUE if the filename at 'name' has an acceptable length. */
/* If it isn't acceptable, set error for the calling process and   */
/* return FALSE.                                                   */
/*-----------------------------------------------------------------*/
static bool8 nlcheck(char *name)
{
    uint32 i;

    for(i=0;i<LF_NAME_LEN;i++)
	if (name[i] == NULLCH)
	    break;
    if (i == 0) {
	proctab[currpid].errno = ENOENT;
	return FALSE;
    }
    if (i >= LF_NAME_LEN) {
	proctab[currpid].errno = ENAMETOOLONG;
	return FALSE;
    }
    return TRUE;
}

/*--------------------------------------------------------------*/
/* Truncate the closed file whose entry is pointed to by ldptr. */
/* Return 1 on success. Otherwise set errno and return 0.       */
/* The directory mutex is assumed to be held.                   */
/*--------------------------------------------------------------*/
/* XXX Can this ever return 0? */
static int lftrunc2(struct ldentry *ldptr)
{
    struct lfiblk iblock;		/* buffer for one index block */
    ibid32 ifree;			/* start of index blk free list */
    ibid32 firstib;			/* first index blk of the file */
    ibid32 nextib;			/* walk list of file's index blks */
    dbid32 nextdb;			/* next data block to free */
    int32 i;				/* index data blks for index blk */

    if (ldptr->ld_size == 0)		/* is file already empty? */
	return OK;			/* if so, we're done */

    ifree = Lf_data.lf_dir.lfd_ifree;	/* ID of 1st index blk on free list */
    firstib = ldptr->ld_ilist;		/* save file's first i-block index */

    for(nextib = firstib; nextib != ifree; nextib = iblock.ib_next) {
	lfibget(Lf_data.lf_dskdev, nextib, &iblock);	/* get i-block */

	for(i=0;i<LF_IBLEN;i++) {	/* free each d-blk in the i-blk */
	    nextdb = iblock.ib_dba[i];	    /* free the data blk */
	    if (nextdb != LF_DNULL)
		lfdbfree(Lf_data.lf_dskdev,nextdb);
	    iblock.ib_dba[i] = LF_DNULL;    /* clear iblk entry for data blk */
	}

	iblock.ib_offset = 0;		/* for debugging ease */

	if (iblock.ib_next == LF_INULL)	/* make last index block point */
	    iblock.ib_next = ifree;	/* to the current free list.   */

	lfibput(Lf_data.lf_dskdev,nextib,&iblock);	/* write to disk */
    }
    Lf_data.lf_dir.lfd_ifree = firstib;
    Lf_data.lf_dirdirty = TRUE;

    return 1;
}

devcall	lfscontrol (
	 struct dentry	*devptr,	/* Entry in device switch table	*/
	 int32	func,			/* A control function		*/
	 int32	arg1,			/* Argument #1			*/
	 int32	arg2			/* Argument #2			*/
	)
{
	int32	retval;			/* Return value from func. call	*/
	char	*old, *new;		/* old/new file names */
	struct	lfdir *dirptr;		/* Ptr to in-memory directory */
	struct ldentry *ldptr;		/* ptr to directory entry */
	char *nam, *cmp;		/* ptr to names */
	int32	dndx;			/* directory index */
	bool8	found;			/* name found? */
	int32	i;			/* index */

	switch (func) {

	    /*-------------------------------------------------------*/
	    /* arg1 is a 0-origin index to the directory. If it does */
	    /* identify an existing directory, it is copied to the   */
	    /* buffer pointed to by arg2, and OK is returned. If it  */
	    /* does not index a directory entry, SYSERR is returned  */
	    /* with errno set to ENOENT.			     */
	    /* XXX What if the LFS doesn't exist?		     */
	    /* XXX Should we check the arg2 pointer for validity?    */
	    /*-------------------------------------------------------*/
	    case LF_CTL_GETDE:
		/*------------------------------------*/
		/* Ensure the directory is in memory. */
		/*------------------------------------*/
		wait(Lf_data.lf_mutex);
		dirptr = &Lf_data.lf_dir;
		if (Lf_data.lf_dirpresent == FALSE) {
		    retval = read(Lf_data.lf_dskdev,(char *)dirptr,LF_AREA_DIR);
		    if (retval == SYSERR ) {
			signal(Lf_data.lf_mutex);
			return SYSERR;
		    }
		    Lf_data.lf_dirpresent = TRUE;
		}

		/*------------------------------------------*/
		/* Verify the index is in the proper range. */
		/*------------------------------------------*/
		if (arg1 < 0 || arg1 >= LF_NUM_DIR_ENT) {
		    proctab[currpid].errno = ENOENT;
		    signal(Lf_data.lf_mutex);
		    return SYSERR;
		}

		/*-----------------*/
		/* Copy the entry. */
		/*-----------------*/
		memcpy((void *)arg2,
		    (const void *)&Lf_data.lf_dir.lfd_files[arg1],
		    sizeof(struct ldentry));
		signal(Lf_data.lf_mutex);
		return OK;

	    /*---------------------------------------------------------*/
	    /* arg1 is a pointer to a file name. That file is deleted. */
	    /* If the file does not exist, errno is set to ENOENT and  */
	    /* SYSERR is returned.				       */
	    /* errno is set to ENAMETOOLONG and SYSERR is returned if  */
	    /* the filename is too long.                               */
	    /*---------------------------------------------------------*/
	    case LF_CTL_DEL:
		/*-----------------------------------------------------*/
		/* We REALLY need to verify the file is not open here. */
		/* Deleting an open file is bad news.                  */
		/*-----------------------------------------------------*/
		/*-----------------------*/
		/* Check length of name. */
		/*-----------------------*/
		old = (char *)arg1;
		if (nlcheck(old) == FALSE) {
		    proctab[currpid].errno = ENAMETOOLONG;
		    return SYSERR;
		}

		/*------------------------------------*/
		/* Ensure the directory is in memory. */
		/*------------------------------------*/
		wait(Lf_data.lf_mutex);

		dirptr = &Lf_data.lf_dir;
		if (Lf_data.lf_dirpresent == FALSE) {
		    retval = read(Lf_data.lf_dskdev,(char *)dirptr,LF_AREA_DIR);
		    if (retval == SYSERR ) {
			signal(Lf_data.lf_mutex);
			return SYSERR;
		    }

		    /*---------------------------------------------------*/
		    /* Verify the magic number. If not present, then     */
		    /* mark the directory not present and return SYSERR. */
		    /*---------------------------------------------------*/
		    if (dirptr->lfd_magic[0] != 'L' ||
			dirptr->lfd_magic[1] != 'F' ||
			dirptr->lfd_magic[2] != 'S' ||
			dirptr->lfd_magic[3] != 'Y') {

			Lf_data.lf_dirpresent = FALSE;
			proctab[currpid].errno = EBADMAGIC;
			signal(Lf_data.lf_mutex);
			return SYSERR;
		    }

		    Lf_data.lf_dirpresent = TRUE;
		}

		/*----------------------------------------------*/
		/* Verify the filename exists in the directory. */
		/*----------------------------------------------*/
		found = FALSE;
		for (i=0; i<dirptr->lfd_nfiles; i++) {

		    ldptr = &dirptr->lfd_files[i];
		    nam = old;
		    cmp = ldptr->ld_name;
		    while(*nam != NULLCH) {
                        if (*nam != *cmp) {
                                break;
                        }
                        nam++;
                        cmp++;
		    }
		    if ( (*nam==NULLCH) && (*cmp==NULLCH) ) { /* Name found */
                        found = TRUE;
                        break;
		    }
		}

		if (found == FALSE) {
		    proctab[currpid].errno = ENOENT;
		    signal(Lf_data.lf_mutex);
		    return SYSERR;		/* old filename not found */
		}

		/*-------------------------------------------------------*/
		/* Reclaim blocks used by the file. This is essentially  */
		/* the same work done by lftrucate, except that requires */
		/* that the file be open. So we use a static version of  */
		/* the appropriate part of the code here to reclaim the  */
		/* index and data blocks.                                */
		/*-------------------------------------------------------*/
		if (lftrunc2(ldptr) == 0) {	/* errno set on return */
		    signal(Lf_data.lf_mutex);
		    return SYSERR;
		}

		/*-------------------------------------------------*/
		/* If this wasn't the last entry in the directory, */
		/* move the last entry to the slot occupied by the */
		/* file we just deleted, and clear the last entry. */
		/* Otherwise just clear the last directory entry.  */
		/*-------------------------------------------------*/
		if (i < dirptr->lfd_nfiles-1) {
		    struct ldentry *ldptr2;	/* ptr to last dir entry */

		    ldptr2 = &dirptr->lfd_files[dirptr->lfd_nfiles-1];
		    memcpy((void *)ldptr, (const void *)ldptr2,
			sizeof(struct ldentry));
		    memset((char *)ldptr2,NULLCH,sizeof(struct ldentry));
		} else {
		    memset((char *)ldptr,NULLCH,sizeof(struct ldentry));
		}

		/*------------------------*/
		/* Adjust the file count. */
		/*------------------------*/
		Lf_data.lf_dir.lfd_nfiles--;

		/*-----------------------------------------------*/
		/* Mark the in-memory directory structure dirty. */
		/*-----------------------------------------------*/
		Lf_data.lf_dirdirty = TRUE;

	        signal(Lf_data.lf_mutex);
		return OK;

	    /*---------------------------------------------------*/
	    /* arg1 points to an "old" filename, and args points */
	    /* to a "new" filename. If either name is 0-length,  */
	    /* errno is set to ENOENT and SYSERR is returned. If */
	    /* a name is too long, errno is set to ENAMETOOLONG  */
	    /* and SYSERR is returned. If the "new" filename is  */
	    /* already used, errno is set to EEXIST and SYSERR   */
	    /* is returned. Otherwise the name of the "old" file */
	    /* is changed to "new".				 */
	    /*---------------------------------------------------*/
	    case F_CTL_RENAME:
		/*-----------------------------------------------*/
		/* XXX We may later want to verify that the file */
		/* XXX being renamed is not open. But why would  */
		/* XXX the file being open be an error?          */
		/*-----------------------------------------------*/
		/*----------------------------------------------*/
		/* Check length of old/new names. Each must be  */
		/* between 1 and LF_NAME_LEN-1 characters long. */
		/*----------------------------------------------*/
		old = (char *)arg1;
		if (nlcheck(old) == FALSE) {
		    proctab[currpid].errno = ENAMETOOLONG;
		    return SYSERR;
		}

		new = (char *)arg2;
		if (nlcheck(new) == FALSE) {
		    proctab[currpid].errno = ENAMETOOLONG;
		    return SYSERR;
		}

		/*------------------------------------*/
		/* Ensure the directory is in memory. */
		/*------------------------------------*/
		wait(Lf_data.lf_mutex);
		dirptr = &Lf_data.lf_dir;
		if (Lf_data.lf_dirpresent == FALSE) {
		    retval = read(Lf_data.lf_dskdev,(char *)dirptr,LF_AREA_DIR);
		    if (retval == SYSERR ) {
			signal(Lf_data.lf_mutex);
			return SYSERR;
		    }
		    Lf_data.lf_dirpresent = TRUE;
		}

		/*----------------------------------------------------*/
		/* Verify the "old" filename exists in the directory. */
		/*----------------------------------------------------*/
		found = FALSE;
		for (i=0; i<dirptr->lfd_nfiles; i++) {

		    ldptr = &dirptr->lfd_files[i];
		    nam = old;
		    cmp = ldptr->ld_name;
		    while(*nam != NULLCH) {
                        if (*nam != *cmp) {
                                break;
                        }
                        nam++;
                        cmp++;
		    }
		    if ( (*nam==NULLCH) && (*cmp==NULLCH) ) { /* Name found */
                        found = TRUE;
                        break;
		    }
		}
		if (found == FALSE) {
		    proctab[currpid].errno = ENOENT;
		    signal(Lf_data.lf_mutex);
		    return SYSERR;		/* old filename not found */
		}
		dndx = i;			/* save directory index */

		/*-------------------------------------------*/
		/* Verify the "new" filename does NOT exist. */
		/*-------------------------------------------*/
		found = FALSE;
		for (i=0; i<dirptr->lfd_nfiles; i++) {
		    char *nam, *cmp;		/* ptr to names */
		    struct ldentry *ldptr;	/* ptr to directory entry */

		    ldptr = &dirptr->lfd_files[i];
		    nam = new;
		    cmp = ldptr->ld_name;
		    while(*nam != NULLCH) {
                        if (*nam != *cmp) {
                                break;
                        }
                        nam++;
                        cmp++;
		    }
		    if ( (*nam==NULLCH) && (*cmp==NULLCH) ) { /* Name found */
                        found = TRUE;
                        break;
		    }
		}
		if (found == TRUE) {
		    proctab[currpid].errno = EEXIST;
		    signal(Lf_data.lf_mutex);
		    return SYSERR;		/* new filename exists */
		}

		/*-----------------------------------------*/
		/* Replace the old name with the new name. */
		/*-----------------------------------------*/
		ldptr = &dirptr->lfd_files[dndx];
		nam = ldptr->ld_name;
		while (*new)
		    *nam++ = *new++;
		*nam = NULLCH;
		Lf_data.lf_dirdirty = TRUE;
		signal(Lf_data.lf_mutex);
		return OK;

	    /*-----------------------------------*/
	    /* Verify a local filesystem exists. */
	    /* Returns OK or SYSERR.             */
	    /*-----------------------------------*/
	    case LF_CTL_EXIST:
		/*------------------------------------*/
		/* Ensure the directory is in memory. */
		/*------------------------------------*/
		wait(Lf_data.lf_mutex);
		dirptr = &Lf_data.lf_dir;
		if (Lf_data.lf_dirpresent == FALSE) {
		    retval = read(Lf_data.lf_dskdev,(char *)dirptr,LF_AREA_DIR);
		    if (retval == SYSERR ) {
			signal(Lf_data.lf_mutex);
			return SYSERR;
		    }
		    Lf_data.lf_dirpresent = TRUE;
		}

		/*---------------------------------------------------*/
		/* Verify the magic number. If not present, then     */
		/* mark the directory not present and return SYSERR. */
		/*---------------------------------------------------*/
		if (dirptr->lfd_magic[0] != 'L' ||
		    dirptr->lfd_magic[1] != 'F' ||
		    dirptr->lfd_magic[2] != 'S' ||
		    dirptr->lfd_magic[3] != 'Y') {

		    Lf_data.lf_dirpresent = FALSE;
		    proctab[currpid].errno = EBADMAGIC;
		    signal(Lf_data.lf_mutex);
		    return SYSERR;
		}

		signal(Lf_data.lf_mutex);
		return OK;

	default:
		proctab[currpid].errno = EINVAL;
		return SYSERR;
	}
}
