/*-----------------------------------------------------*/
/* dine.c - demonstrate swait and ssignal system calls */
/*-----------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* This program is a "solution" to the classic dining philosopher's problem  */
/* for Xinu with the swait and ssignal system calls.                         */
/*                                                                           */
/* The program is wired for 4 philosophers, but this can be changed by       */
/* changing the definition of NPHIL. Logically there should be at least 4    */
/* philosophers.                                                             */
/*                                                                           */
/* The program is invoked as follows:                                        */
/*	dine    NMEALS                                                       */
/* where NMEALS is the number of times each philosopher will eat before it   */
/* terminates (AFTER giving up the forks it used the last time).             */
/*                                                                           */
/* As each philosopher is created, it obtains its "philosopher number" from  */
/* the argument to phil_func provided by create. Once it has saved this, a   */
/* newly-created philosopher process signals the 'term' semaphore.           */
/*                                                                           */
/* The main process, after creating each philosopher process, will wait on   */
/* the term semaphore to guarantee it waits until the philosopher process    */
/* has saved its philosopher number. It then changes the loop index (i)      */
/* that is also the philosopher number.                                      */
/*                                                                           */
/* As each philosopher process completes, it signals the term semaphore,     */
/* which was initialized to 0 by the parent. After getting all the           */
/* philosophers started, the parent will do one wait on the term semaphore   */
/* for each philosopher. Thus the parent will not terminate until            */
/* each of the philosophers has terminated.                                  */
/*                                                                           */
/* The length of time each philosopher thinks and eats is determined using   */
/* random numbers in the range RLOW to RHIGH to represent a number of        */
/* milliseconds.                                                             */
/*---------------------------------------------------------------------------*/
#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NPHIL 4			/* number of philosophers */

/*---------------------------------------------------------*/
/* Define SHOWTIME to display thinking and sleeping times. */
/*---------------------------------------------------------*/
#define SHOWTIME

#define RLOW 100		/* eat or think at least 100 msec */
#define RHIGH 1000		/* and no more than 1000 msec */

pid32 phil[NPHIL];		/* philosopher process IDs */
sid32 sem[NPHIL];		/* fork pair semaphores */
sid32 term;			/* term semaphore */
uint32 eaten[NPHIL];		/* # of meals each philosopher ate */

int nmeals;			/* most meals any philosopher eats */

/*-------------------------------------------------------------*/
/* Acquire the two forks needed for philosopher 'pnum' to eat. */
/*-------------------------------------------------------------*/
void take_forks(int pnum)
{
    swait(sem[pnum], sem[(pnum+1)%NPHIL]);
}

/*----------------------------------------------------------------*/
/* Philosopher 'pnum' finishes eating and relinquishes its forks. */
/*----------------------------------------------------------------*/
void put_forks(int pnum)
{
    ssignal(sem[pnum], sem[(pnum+1)%NPHIL]);
}

/*----------------------------------------------------*/
/* Each philosopher thinks, eats, thinks, eats, ...   */
/* This function is the body of a process.            */
/* The value of arg is the philosopher number (from   */
/* 0 to NPHIL-1). Once the process has obtained its   */
/* philosopher number, it signals the term semaphore. */
/*----------------------------------------------------*/
void phil_func(int32 arg)
{
    int phil_num;		/* my philosopher number */
    int my_meals;		/* # of times I've eaten */
    int eat_time, think_time;

    /*-----------------------------*/
    /* Get the philosopher number. */
    /*-----------------------------*/
    phil_num = arg;
    signal(term);			/* tell main we got it */

    my_meals = 0;

    while (my_meals < nmeals) {		/* while I haven't eaten enuf times */

	/*----------*/
	/* THINK... */
	/*----------*/
	think_time = RLOW + rand() % (RHIGH - RLOW);
#ifdef SHOWTIME
	printf("P%d thinking for %d msec.\n", phil_num, think_time);
#else
	printf("P%d thinking.\n", phil_num);
#endif
	sleepms(think_time);

	take_forks(phil_num);

	/*--------*/
	/* EAT... */
	/*--------*/
	eat_time = RLOW + rand() % (RHIGH - RLOW);
#ifdef SHOWTIME
	printf("P%d eating meal # %d for %d msec.\n",
	    phil_num, my_meals+1, eat_time);
#else
	printf("P%d eating meal # %d.\n",phil_num, my_meals+1);
#endif
	sleepms(eat_time);

	put_forks(phil_num);

	my_meals++;
    }

#ifdef SHOWTIME
    printf("P%d done.\n", phil_num);
#else
    printf("P%d done.\n", phil_num);
#endif

    eaten[phil_num] = my_meals;

    signal(term);			/* tell the parent we're done */
    /*----------------------------------------------------------------*/
    /* XXX Perhaps we should use a different semaphore here. It is    */
    /* XXX unlikely, but possible, that an philosopher that's created */
    /* XXX early could finish before the other philosophers are       */
    /* XXX created!                                                   */
    /*----------------------------------------------------------------*/
}

int xsh_dine(int argc, char *argv[])
{
    int i;

    if ((argc == 2 && !strcmp(argv[1],"--help")) || argc != 2) {
	printf("Usage: dine NMEALS\n");
	printf("Read the program comments for details.\n");
	return 0;
    }
    nmeals = atoi(argv[1]);
    if (nmeals < 1) {
	printf("Error: NMEALS must be between at least 1.\n");
	printf("Read the program comments for details.\n");
	return 0;
    }

    /* Seed the random number generator. */
    /* XXX */

    /*----------------------------------------------------------*/
    /* Guarantee our priority is high enough so we can run the  */
    /* philosopher processes at a lower priority. We do this by */
    /* using getsysparm to get the lowest possible process      */
    /* priority, and then setting our priority to one larger.   */
    /* The philosopher's priorities will be the lowest possible */
    /* priority.                                                */
    /*                                                          */
    /* There is still a potential "gotcha." If someone should   */
    /* define the maximum and minumum priorities as equal, then */
    /* the effort to set our (parent) priority to something     */
    /* larger than the philosophers will fail. We don't worry   */
    /* about that here, except to document that weird case.     */
    /*----------------------------------------------------------*/

    /*-----------------------------------------*/
    /* Allocate and initialize the semaphores. */
    /*-----------------------------------------*/
    for (i=0;i<NPHIL;i++) {		/* first we do the fork semaphores */
	sem[i] = semcreate(1);
	if (sem[i] == SYSERR) {
	    printf("Error: cannot create semaphore %d\n", i+1);
	    while (--i >= 0)		/* cleanup */
		semdelete(sem[i]);
	    return 0;			/* we've failed... */
	}
    }

    /*-------------------------*/
    /* Now the term semaphore. */
    /*-------------------------*/
    term = semcreate(0);
    if (term == SYSERR) {
	printf("Error: cannot create term semaphore\n");
	for(i=0;i<NPHIL;i++)
	    semdelete(sem[i]);
	return 0;			/* we've failed... */
    }


    /*------------------------------------------------*/
    /* Now let's crank out the philosopher processes. */
    /*------------------------------------------------*/
    for (i=0;i<NPHIL;i++) {
	char name[50];

	sprintf(name,"phil%d",i);		/* create philosoper name */
	phil[i] = create(phil_func, 8192, 50, name, 1, i);
	if (phil[i] == SYSERR) {
	    printf("Trouble creating philosoper %d.\n", i);
	    for(i=0;i<NPHIL;i++)	/* cleanup */
		semdelete(sem[i]);
	    semdelete(term);
	    while(--i > 0)
		kill(phil[i]);
	    return 0;
	}
	resume(phil[i]);
	wait(term);		/* wait for it to get it's number */
    }

    /*---------------------------------------------------------------------*/
    /* Wait for each phlosopher to complete.                               */
    /* For debugging purposes, we include a timeout that is twice as long  */
    /* as NPHIL * 2 * maximum eat/sleep time, so we won't time out waiting */
    /* for the termination semaphore inappropriately. If we do timeout, it */
    /* indicates one or more philosophers are "stuck" and we report their  */
    /* states and kill off the ones that are still "live" before quitting. */
    /*---------------------------------------------------------------------*/
    for(i=0;i<NPHIL;i++)
	wait(term);

    /*-------------------------*/
    /* Cleanup the semaphores. */
    /*-------------------------*/
    for(i=0;i<NPHIL;i++)
	semdelete(sem[i]);
    semdelete(term);

    /*-----------------------------------------------------*/
    /* Display total number of meals each philosopher ate. */
    /*-----------------------------------------------------*/
    printf("Phil#");
    for(i=0;i<NPHIL;i++)
	printf("\t%d",i);
    printf("\n");
    printf("#meals");
    for(i=0;i<NPHIL;i++)
	printf("\t%d",eaten[i]);
    printf("\n");

    return 0;
}
