/* showmema.c - showmema */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  showmema  -  Show addresses for beginning and end of text, data, bss
 *------------------------------------------------------------------------
 */
void    showmema(void)
{
	intmask	mask;			/* Saved interrupt mask		*/

	mask = disable();

	kprintf("\n");
	kprintf("Xinu segment addresses (from showmema)...\n");
	kprintf("   text: 0x%08x through 0x%08x\n",
	    (uint32)&text, (uint32)&etext - 1);
	kprintf("   data: 0x%08x through 0x%08x\n",
	    (uint32)&data, (uint32)&edata - 1);
	kprintf("   bss: 0x%08x through 0x%08x\n",
	    (uint32)&bss, (uint32)&ebss - 1);
	kprintf("\n");

	restore(mask);
}
