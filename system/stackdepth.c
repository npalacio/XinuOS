#include <xinu.h>

int stackdepth() {

	struct procent *proc = &proctab[currpid];

	unsigned long *top_esp;
	unsigned long *top_ebp;

	unsigned long *sp;
	unsigned long *fp;

    int count = 0;

	asm("movl %%esp, %0" : "=r" (top_esp));
	asm("movl %%ebp, %0" : "=r" (top_ebp));

	sp = top_esp;
	fp = top_ebp;

	while (sp < (unsigned long *)proc->prstkbase) {

		while (sp < fp) {
			sp++;
		}

		fp = (unsigned long *) *(sp++);
		count++;
		sp++;

	}

	return count;
}
