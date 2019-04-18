// Group Members: Nicholas Palacio, Seth Redwine, Jeffrey Allen

#include <xinu.h>

/*---------------------------------------------------------*/
/* ssignal - Simultaneous signal of two semaphores.        */
/*---------------------------------------------------------*/

syscall ssignal(
        sid32   semA,   
        sid32   semB
        )
{
    intmask mask;           /* Saved interrupt mask         */
    struct procent *prptr;  /* Ptr to process' table entry  */
    struct sentry *semptrA; /* Ptr to sempahore table entry */
    struct sentry *semptrB; /* Ptr to sempahore table entry */

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

    // Start resched_cntl
    resched_cntl(DEFER_START);

    // Increment the counts of each semaphore and
    // move longest waiting process to ready state
    if ((semptrA->scount++) < 0) {
        ready(dequeue(semptrA->squeue));
    }

    if ((semptrB->scount++) < 0) {
        ready(dequeue(semptrB->squeue));
    }

    // End resched_cntl
    resched_cntl(DEFER_STOP);
    
    restore(mask);
    return OK;
}
