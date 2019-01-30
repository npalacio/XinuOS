#include <xinu.h>

/*---------------------------------------------------------*/
/* This is an empty shell for a system call named ssignal. */
/*---------------------------------------------------------*/

syscall ssignal(
        sid32   semA,   
        sid32   semB
        )
{
    return OK;
}
