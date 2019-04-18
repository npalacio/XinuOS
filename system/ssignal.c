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
    return OK;
}
