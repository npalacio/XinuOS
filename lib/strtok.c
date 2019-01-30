/* strtok.c - strtok */

/*------------------------------------------------------------------------
 *  strtok - find next token in s1[i] delimited by s2[]
 *------------------------------------------------------------------------
 */

#include <xinu.h>

char *strtok(char *s1, const char *s2)
{
    char *sbegin, *send;
    static char *ssave = "";		/* starting point if s1 is NULL */

    /*-----------------------------------------------*/
    /* If s1 is not NULL, start there. Otherwise use */
    /* the saved starting point after the last call. */
    /*-----------------------------------------------*/
    sbegin = (s1) ? s1 : ssave;
    if (sbegin == NULL) {
        ssave = "";
        return NULL;
    }

    /*--------------------------------------------*/
    /* Skip an initial run of characters from s2. */
    /*--------------------------------------------*/
    sbegin += strspn(sbegin, s2);

    /*----------------------------------------------------*/
    /* If we're now at the end of the string, we're done. */
    /* Reset our default string and return NULL.          */
    /*----------------------------------------------------*/
    if (*sbegin == '\0') {
        ssave = "";
        return NULL;
    }

    /*---------------------------------------------------------*/
    /* Otherwise, find the first character after sbegin in s2. */
    /*---------------------------------------------------------*/
    send = strpbrk(sbegin, s2);

    /*-------------------------------------------------*/
    /* If it's not a null, then replace it with a null */
    /* and advance the end pointer to the next byte.   */
    /*-------------------------------------------------*/
    if (send != NULL)
        *send++ = '\0';

    /*-------------------------------*/
    /* Save the next starting point. */
    /*-------------------------------*/
    ssave = send;

    /*-----------------------------------------*/
    /* Finally, return the start of the token. */
    /*-----------------------------------------*/
    return sbegin;
}
