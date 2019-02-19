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
    printf("Text:\n\tStart:  0x%08X\n\tEnd:    0x%08X\n\tLength: %10d\n\tByte 1:       0x%02X\n\tByte 2:       0x%02X\n\tByte 3:       0x%02X\n\tByte 4:       0x%02X\n\n", (uint32)&text, (uint32)&etext - 1, (uint32)&etext - (uint32)&text, *((unsigned char *)(&text)[3]), *((unsigned char *)(&text)[2]), *((unsigned char *)(&text)[1]), *((unsigned char *)(&text)[0]));


    printf("Data:\n\tStart:  0x%08X\n\tEnd:    0x%08X\n\tLength: %10d\n\tByte 1:       0x%02X\n\tByte 2:       0x%02X\n\tByte 3:       0x%02X\n\tByte 4:       0x%02X\n\n", (uint32)&data, (uint32)&edata - 1, (uint32)&edata - (uint32)&data), *((unsigned char *)(&data)[3]), *((unsigned char *)(&data)[2]), *((unsigned char *)(&data)[1]), *((unsigned char *)(&data)[0]);


    printf("BSS:\n\tStart:  0x%08X\n\tEnd:    0x%08X\n\tLength: %10d\n\tByte 1:       0x%02X\n\tByte 2:       0x%02X\n\tByte 3:       0x%02X\n\tByte 4:       0x%02X\n\n", (uint32)&edata, (uint32)&ebss - 1, (uint32)&ebss - (uint32)&edata, *((unsigned char *)(&edata)[3]), *((unsigned char *)(&edata)[2]), *((unsigned char *)(&edata)[1]), *((unsigned char *)(&edata)[0]));
}
