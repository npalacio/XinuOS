/* qsort.c - qsort */

/*----------------------------------------------------------*/
/* XXX This is a replacement for the original qsort.c file. */
/* XXX The qsort function it provided was not compatible    */
/* XXX with the qsort function in the standard C library.   */
/* XXX This is the version from Plauger's book, with patch. */
/* XXX -- Stan Wileman					    */
/*----------------------------------------------------------*/

/*------------------------------------------------------------------------
 *  qsort  -  Sorts an array using quicksort.
 *------------------------------------------------------------------------
 */

#include <xinu.h>
typedef unsigned int size_t;
#include <stdlib.h>

#define MAX_BUF 256

void qsort(void *base, size_t n, size_t size,
		int32 (*cmp)(const void *, const void *))
{
    while (1 < n) {			/* is there any need to sort? */
	size_t i = 0;
	size_t j = n - 1;
	char *qi = base;
	char *qj = qi + size * j;
	char *qp = qj;

	while (i < j) {			/* partition about pivot */
	    while (i < j && (*cmp)(qi, qp) <= 0)
		++i, qi += size;
	    while (i < j && (*cmp)(qp, qj) <= 0)
		--j, qj -= size;
	    if (i < j) {
		char buf[MAX_BUF];
		char *q1 = qi;
		char *q2 = qj;
		size_t m, ms;

		for (ms = size; 0 < ms; ms -= m, q1 += m, q2 -= m) {
		    m = ms < sizeof(buf) ? ms : sizeof(buf);
		    memcpy(buf, q1, m);
		    memcpy(q1, q2, m);
		    memcpy(q2, buf, m);
		}
		++i, qi += size;
		--j, qj -= size;
	    }
	}

	/* PATCH */
        if (i == j && qi != qp && (*cmp)(qi,qp) <= 0)
            ++i, qi += size;
	/* PATCH END */

	if (qi != qp) {
	    char buf[MAX_BUF];
	    char *q1 = qi;
	    char *q2 = qp;
	    size_t m, ms;

	    for (ms = size; 0 < ms; ms -= m, q1 += m, q2 -= m) {
		m = ms < sizeof(buf) ? ms : sizeof(buf);
		memcpy(buf, q1, m);
		memcpy(q1, q2, m);
		memcpy(q2, buf, m);
	    }
	}

	j = n - i;
	if (j < i) {
	    if (1 < j)
		qsort(qi, j, size, cmp);
	    n = i;
	} else {
	    if (1 < i)
		qsort(base, i, size, cmp);
	    base = qi;
	    n = j;
	}
    }
}
