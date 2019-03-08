/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>


extern uint32 nsaddr;
extern void printsegaddress(void);

process	main(void)
{
	/* Start the network, if possible */

	netstart();

	nsaddr = 0x800a0c10;

    printsegaddress();
    func1();

	recvclr();

    //printsegaddress();

    kprintf("Ready to read character...");
    char character[1];
    while (TRUE) {
        read(CONSOLE, character, sizeof(character));
        if (*character == 0x01) {
            kprintf("Control-A character read successfully!");
        }
    }
	//resume(create(shell, 8192, 50, "shell", 1, CONSOLE));

	/* Wait for shell to exit and recreate it */

	while (TRUE) {
		receive();
		sleepms(200);
		kprintf("\n\nMain process recreating shell\n\n");
		resume(create(shell, 4096, 20, "shell", 1, CONSOLE));
	}

	return OK;		/* never reached */
}
