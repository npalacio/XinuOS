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
    struct  procent *prptr;     /* Ptr to process' table entry  */
    struct  sentry *semptrA;     /* Ptr to sempahore table entry */
    struct  sentry *semptrB;     /* Ptr to sempahore table entry */

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

    // If both semaphores are 'good to go', decrement, restore mask and return from system call
    if(semptrA->scount > 0 && semptrB->scount > 0) {
        semptrA->scount--;
        semptrB->scount--;
        restore(mask);
        return OK; 
    } else {
        // One of the semaphores is not 'good to go'
        if(semptrA->scount <= 0) {
            // Block our process for this semaphore
            semptrA->scount--;
            prptr = &proctab[currpid];
            prptr->prstate = PR_WAIT;   /* Set state to waiting */
            prptr->prsem = semA;     /* Record semaphore ID  */
            enqueue(currpid,semptrA->squeue);/* Enqueue on semaphore */
            resched();          /*   and reschedule */

            // When we get back lets run through everything again
                // We shouldn't hit this block twice in a row since resched will only return when our process has been run again after the semA became 'good to go'
            return swait(semA, semB);
        }
        if(semptrB->scount <= 0) {
            // Block our process for this semaphore
            semptrB->scount--;
            prptr = &proctab[currpid];
            prptr->prstate = PR_WAIT;   /* Set state to waiting */
            prptr->prsem = semB;     /* Record semaphore ID  */
            enqueue(currpid,semptrB->squeue);/* Enqueue on semaphore */
            resched();          /*   and reschedule */

            // When we get back lets run through everything again
                // We shouldn't hit this block twice in a row since resched will only return when our process has been run again after the semA became 'good to go'
            return swait(semA, semB);
        }
    }
    // If either semaphore is not 'good to go', block the process
    return OK; 
}
