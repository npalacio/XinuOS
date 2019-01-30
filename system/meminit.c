/* meminit.c - memory bounds and free list init */

#include <xinu.h>

/* Memory bounds */

void	*minheap;		/* Start of heap			*/
void	*maxheap;		/* Highest valid heap address		*/

/* Memory map structures */

uint32	bootsign = 1;		/* Boot signature of the boot loader	*/

struct	mbootinfo *bootinfo = (struct mbootinfo *)1;	
				/* Base address of the multiboot info	*/
				/*  provided by GRUB, initialized just	*/
				/*  to guarantee it is in the DATA	*/
				/*  segment and not the BSS		*/

/* Segment table structures */

/* Segment Descriptor */

struct __attribute__ ((__packed__)) sd {
					/* second doubleword of descriptor */
	unsigned short	sd_lolimit;	/* bits 0-15 of segment limit */
	unsigned short	sd_lobase;	/* bits 0-15 of segment base addr */

					/* first doubleword of descriptor */
	unsigned char	sd_midbase;	/* bits 7-0: seg. base addr (23-16) */
	unsigned char   sd_access;	/* bits 11-8: seg. type */
					/* bit 12: descriptor type: */
					    /* 0 - system, 1 = code or data */
					/* bits 14-13: descr. priv. level */
					/* bit 15: seg. present */
	unsigned char	sd_hilim_fl;	/* bits 19-16: seg. lim. 19-16 */
					/* bit 20: unused */
					/* bit 21: must be 0 */
					/* bit 22: def. op size (0=16, 1=32) */
					/* bit 23: granularity */
	unsigned char	sd_hibase;	/* bits 31-24: seg. base addr 31-24 */
};

/* # of GDT entries increased 8 to 11 (see step 3 in 8.9.2 Intel sysprog) */
#define	NGD			11	/* # of global descr entries	*/
#define FLAGS_GRANULARITY	0x80	/* if set, seg lim in 4K byte units */
#define FLAGS_SIZE		0x40	/* if seg, def op size = 32 bits */
#define	FLAGS_SETTINGS		(FLAGS_GRANULARITY | FLAGS_SIZE)

/*----------------------------------------------------------------------*/
/* Prototypes for the entries in the global descriptor table.		*/
/* Common settings are as follows:					*/
/*     sd_access:							*/
/*	P (segment present flag, bit 15) = 1				*/
/*	DPL (descriptor priv. level, bits 14-13) = 0 (most privileged)	*/
/*	S (descriptor type,, bit 12) = 1 (code or data)			*/
/*	TYPE (seg type, bits 11-8):					*/
/*	    1010 = 0xa = code segment, execute/read access		*/
/*	    0010 = 0x2 = data segment, read/write			*/
/*----------------------------------------------------------------------*/
struct sd gdt_copy[NGD] = {
/*   sd_lolimit  sd_lobase   sd_midbase  sd_access   sd_hilim_fl sd_hibase */
/* 0th entry NULL */
{            0,          0,           0,         0,            0,        0, },

/* 1st, Kernel Code Segment */
{       0xffff,          0,           0,      0x9a,         0xcf,        0, },

/* 2nd, Kernel Data Segment */
{       0xffff,          0,           0,      0x92,         0xcf,        0, },

/* 3rd, Kernel Stack Segment */
{       0xffff,          0,           0,      0x92,         0xcf,        0, },

/* 4st, Bootp Code Segment */
{       0xffff,          0,           0,      0x9a,         0xcf,        0, },

/* 5th, Code Segment for BIOS32 request */
{       0xffff,          0,           0,      0x9a,         0xcf,        0, },

/* 6th, Data Segment for BIOS32 request */
{       0xffff,          0,           0,      0x92,         0xcf,        0, },

/* 7th, Code Segment for reboot; seg. limit is 0xffff */
{       0xffff,          0,           0,      0x9a,         0x40,        0, },

/* 8th, Data Segment for reboot; seg. limit is 0xffff */
{       0xffff,          0,           0,      0x92,         0x40,        0, },

/* 9th, Stack Segment for reboot */
{       0xffff,          0,           0,      0x92,         0x40,        0, },

};

extern	struct	sd gdt[];	/* Global segment table			*/

/*------------------------------------------------------------------------
 * meminit - initialize memory bounds and the free memory list
 *------------------------------------------------------------------------
 */
void	meminit(void) {

	struct	memblk	*memptr;	/* Ptr to memory block		*/
	struct	mbmregion	*mmap_addr;	/* Ptr to mmap entries	*/
	struct	mbmregion	*mmap_addrend;	/* Ptr to end of mmap region */
	struct	memblk	*next_memptr;	/* Ptr to next memory block	*/
	uint32	next_block_length;	/* Size of next memory block	*/
	
	mmap_addr = (struct mbmregion*)NULL;
	mmap_addrend = (struct mbmregion*)NULL;

	/* Initialize the free list */
	memptr = &memlist;
	memptr->mnext = (struct memblk *)NULL;
	memptr->mlength = 0;
	
	/* Initialize the memory counters */
	/*    Heap starts at the end of Xinu image */
	minheap = (void*)&end;
	maxheap = minheap;
	
	/* Check if Xinu was loaded using the multiboot specification	*/
	/*   and a memory map was included				*/
	if(bootsign != MULTIBOOT_SIGNATURE) {
		panic("could not find multiboot signature");
	}
	if(!(bootinfo->flags & MULTIBOOT_BOOTINFO_MMAP)) {
		panic("no mmap found in boot info");
	}
	
	/* Get base address of mmap region (passed by GRUB) */
	mmap_addr = (struct mbmregion*)bootinfo->mmap_addr;
		
	/* Calculate address that follows the mmap block */
	mmap_addrend = (struct mbmregion*)((uint8*)mmap_addr
	    + bootinfo->mmap_length);

	/* Read mmap blocks and initialize the Xinu free memory list	*/
	while(mmap_addr < mmap_addrend) {

		/* If block is not usable, skip to next block */
		if(mmap_addr->type != MULTIBOOT_MMAP_TYPE_USABLE) {
			mmap_addr = (struct mbmregion*)((uint8*)mmap_addr
			    + mmap_addr->size + 4);
			continue;
		}
			
		if((uint32)maxheap < ((uint32)mmap_addr->base_addr
					+ (uint32)mmap_addr->length)) {
			maxheap = (void*)((uint32)mmap_addr->base_addr
					+ (uint32)mmap_addr->length);
		}

		/* Ignore memory blocks within the Xinu image */
		if((mmap_addr->base_addr + mmap_addr->length) <
		    ((uint32)minheap)) {
			mmap_addr = (struct mbmregion*)
			    ((uint8*)mmap_addr + mmap_addr->size + 4);
			continue;
		}
		
		/* The block is usable, so add it to Xinu's memory list */

		/* This block straddles the end of the Xinu image */
		if((mmap_addr->base_addr <= (uint32)minheap) &&
		  ((mmap_addr->base_addr + mmap_addr->length) >
		  (uint32)minheap)) {

			/* First free block: base address is the minheap */
			next_memptr = (struct memblk *)roundmb(minheap);

			/* Subtract Xinu image from length of block */
			next_block_length =
			    (uint32)truncmb(mmap_addr->base_addr
				+ mmap_addr->length
				- (uint32)minheap);
		} else {

			/* Handle free memory block other than the first one */
			next_memptr = (struct memblk *)
			    roundmb(mmap_addr->base_addr);

			/* Initialize the length of the block */
			next_block_length = (uint32)truncmb(mmap_addr->length);
		}
		
		/* Add then new block to the free list */
		memptr->mnext = next_memptr;
		memptr = memptr->mnext;
		memptr->mlength = next_block_length;
		memlist.mlength += next_block_length;

		/* Move to the next mmap block */
		mmap_addr = (struct mbmregion*)
		    ((uint8*)mmap_addr + mmap_addr->size + 4);
	}

	/* End of all mmap blocks, and so end of Xinu free list */
	if(memptr) {
		memptr->mnext = (struct memblk *)NULL;
	}
}


/*------------------------------------------------------------------------
 * setsegs  -  Initialize the global segment table
 *------------------------------------------------------------------------
 */
void	setsegs()
{
	extern int	etext;
	struct sd	*psd;
	uint32		np, ds_end;

	ds_end = 0xffffffff/PAGE_SIZE; /* End page number of Data segment */

	/* Kernel code segment: identity map for addresses 0 to etext */
	psd = &gdt_copy[1];
	np = ((int)&etext - 0 + PAGE_SIZE-1) / PAGE_SIZE;  /* # of code pages */
	psd->sd_lolimit = np;
	psd->sd_hilim_fl = FLAGS_SETTINGS | ((np >> 16) & 0xff);

	/* Kernel data segment */
	psd = &gdt_copy[2];
	psd->sd_lolimit = ds_end;
	psd->sd_hilim_fl = FLAGS_SETTINGS | ((ds_end >> 16) & 0xff);

	/* Kernel stack segment */
	psd = &gdt_copy[3];
	psd->sd_lolimit = ds_end;
	psd->sd_hilim_fl = FLAGS_SETTINGS | ((ds_end >> 16) & 0xff);

	/* Bootp code segment */
	psd = &gdt_copy[4];
	psd->sd_lolimit = ds_end;   /* Allows execution of 0x100000 CODE */
	psd->sd_hilim_fl = FLAGS_SETTINGS | ((ds_end >> 16) & 0xff);

	/* We don't change the segment limit fields in reboot descriptors. */
	memcpy(gdt, gdt_copy, sizeof(gdt_copy));
}
