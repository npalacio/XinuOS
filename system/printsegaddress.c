/* printsegaddress.c - printsegaddress */

/* Group Members: Seth Redwine, Nick Palacio, Jeffrey Allen */

#include <xinu.h>
#include <stdio.h>
#include <string.h>

/*------------------------------------------------------------------------
 * printsegaddress - prints the address of the start and end of the text,
 * data, and bss segments of the XINU OS
 *------------------------------------------------------------------------
 */

void printsegaddress()
{
    char *textData = (void*) (uint32)&text;
    char *dataData = (void*) (uint32)&data;
    char *bssData = (void*) (uint32)&edata;

    printf("Text:\n\tStart: 0x%08x\n\tEnd: 0x%08x\n\tLength: %10d\n\tByte 1: %10d\n\tByte 2: %10d\n\tByte 3: %10d\n\tByte 4: %10d\n\n", (uint32)&text, (uint32)&etext - 1, (uint32)&etext - (uint32)&text, (uint32)&textData[3], (uint32)&textData[2], (uint32)&textData[1], (uint32)&textData[0]);


    printf("Data:\n\tStart: 0x%08X\n\tEnd: 0x%08X\n\tLength: %10d\n\tByte 1: %10d\n\tByte 2: %10d\n\tByte 3: %10d\n\tByte 4: %10d\n\n", (uint32)&data, (uint32)&edata - 1, (uint32)&edata - (uint32)&data), (uint32)&dataData[3], (uint32)&dataData[2], (uint32)&dataData[1], (uint32)&dataData[0];


    printf("BSS:\n\tStart: 0x%08X\n\tEnd: 0x%08x\n\tLength: %10d\n\tByte 1: %10d\n\tByte 2: %10d\n\tByte 3: %10d\n\tByte 4: %10d\n\n", (uint32)&edata, (uint32)&ebss - 1, (uint32)&ebss - (uint32)&edata, (uint32)&bssData[3], (uint32)&bssData[2], (uint32)&bssData[1], (uint32)&bssData[0]);
}
