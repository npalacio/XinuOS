#include <xinu.h>

/*---------------------------------------------------------*/
/* This is an empty shell for a system call named ssignal. */
/*---------------------------------------------------------*/

syscall swait(
        sid32 semA,
        sid32 semB
        )
{
    intmask mask;           /* Saved interrupt mask     */
    struct  sentry *semptr;     /* Ptr to sempahore table entry */

    // Disable interrupts
    mask = disable();

    semptrA = &semtab[semA];
    semptrB = &semtab[semB];
    // Validate the two semaphores
    if(semA == semB) {
        restore(mask);
        return SYSERR;
    }
    if(isbadsem(semA) || isbadsem(semB)) {
        restore(mask);
        return SYSERR;
    }
    if(semptrA->sstate == S_FREE || semptrB->sstate == S_FREE) {
        restore(mask);
        return SYSERR;
    }

    // If both semaphores are 'good to go' restore mask and return from system call
    if(semptr->scount > 0 && ) {
    
    }
    return OK; 
}
