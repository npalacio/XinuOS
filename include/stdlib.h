/**
 * @file stdlib.h
 *
 * $Id: stdlib.h 2051 2009-08-27 20:55:09Z akoehler $
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

int abs(int);
int32 labs(int32);
int atoi(char *);
int32 atol(char *);
void bzero(void *, int);
/* void qsort(char *, unsigned int, int, int (*)(void)); */
void qsort(void *, uint32, uint32, int32(*)(const void *,const void *));
uint32 rand(void);
void srand(unsigned int);
#if 0
void *malloc(uint32);
void free(void *);
#endif
