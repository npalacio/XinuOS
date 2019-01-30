/*--------------------------------------------------------*/
/* XINU: EXPERIMENT WITH READING MODEL SPECIFIC REGISTERS */
/* USAGE:						  */
/*	msr (for now)					  */
/*--------------------------------------------------------*/
/* What follows is information from various Intel docs on */
/* the model-specific registers. The source of each item  */
/* is indicated.                                          */
/*                                                        */
/* From the System Programming Guide                      */
/* ---------------------------------                      */
/* 2.6.7. Reading and Writing Model-Specific Registers    */
/* The RDMSR (read model-specific register) and WRMSR     */
/* (write model-specific register) allow the processor’s  */
/* 64-bit model-specific registers (MSRs) to be read and  */
/* written to, respectively.  The MSR to be read or       */
/* written to is specified by the value in the ECX        */
/* register. The RDMSR instructions reads the value from  */
/* the specified MSR into the EDX:EAX registers; the      */
/* WRMSR writes the value in the EDX:EAX registers into   */
/* the specified MSR. See Section 8.4., “Model-Specific   */
/* Registers (MSRs)”, for more information about the      */
/* MSRs.                                                  */
/*                                                        */
/* From the Intel Quark SoC X1000 manual:                 */
/* --------------------------------------                 */
/*--------------------------------------------------------*/
#include <xinu.h>

/*----------------------------------------------------------------*/
/* For reasons I don't understand, the inline assembler mechanism */
/* (at least the one in the version of gcc I'm using) will not    */
/* handle operands in some cases unless they're of a pointer type */
/* (like *reax). But other versions of gcc seem to do okay.       */
/*----------------------------------------------------------------*/
static uint32 *reax;
static uint32 *rebx;
static uint32 *recx;
static uint32 *redx;

shellcmd xsh_msr(int argc, char *argv[])
{
    uint32 ireax, irebx, irecx, iredx;		/* eax, ebx, ecx, edx as ints */

    if (argc == 2 && strcmp(argv[1],"--help") == 0) {
	printf("Display some machine-specific registers (MSRs)"
	       " for the system.\n");
	printf("Current results of \"cpuid\" with eax = 0 and eax = 1 are"
	       " shown,\n");
	printf("as is the result of \"rdmsr\" with ecx = 0x10.\n");
	printf("\n");
	printf("The only option is \"--help\", which does the obvious.\n");
	return OK;
    }

    /*-------------------------------------------------*/
    /* First check the CPUID instruction with eax = 0. */
    /*-------------------------------------------------*/
    asm("movl $0,%eax\n\tcpuid");
    asm("movl %%eax,%0":"=r"(reax)::);
    asm("movl %%ebx,%0":"=r"(rebx)::);
    asm("movl %%ecx,%0":"=r"(recx)::);
    asm("movl %%edx,%0":"=r"(redx)::);
    ireax = (int)reax;
    irebx = (int)rebx;
    irecx = (int)recx;
    iredx = (int)redx;
    printf("CPUID(0):\n");
    printf("   eax = 0x%08x\n", ireax);
    printf("   ebx = 0x%08x (%c%c%c%c)\n", irebx,
	(char)irebx,
	(char)(irebx>>8),
	(char)(irebx>>16),
	(char)(irebx>>24));
    printf("   ecx = 0x%08x (%c%c%c%c)\n", irecx,
	(char)irecx,
	(char)(irecx>>8),
	(char)(irecx>>16),
	(char)(irecx>>24));
    printf("   edx = 0x%08x (%c%c%c%c)\n", iredx,
	(char)iredx,
	(char)(iredx>>8),
	(char)(iredx>>16),
	(char)(iredx>>24));

    /*---------------------*/
    /* cpuid with eax = 1. */
    /*---------------------*/
    asm("movl $1,%eax\n\tcpuid");
    asm("movl %%eax,%0":"=r"(reax)::);
    asm("movl %%ebx,%0":"=r"(rebx)::);
    asm("movl %%ecx,%0":"=r"(recx)::);
    asm("movl %%edx,%0":"=r"(redx)::);
    ireax = (int)reax;
    irebx = (int)rebx;
    irecx = (int)recx;
    iredx = (int)redx;
    printf("CPUID(1):\n");
    printf("   eax = 0x%08x\n", ireax);
    printf("   ebx = 0x%08x\n", irebx);
    printf("   ecx = 0x%08x\n", irecx);
    printf("   edx = 0x%08x\n", iredx);

    /*---------------------------------------*/
    /* Try RDMSR with ecx = 0x10 (IS32_TSC). */
    /*---------------------------------------*/
    asm("movl $16,%ecx\n\trdmsr");
    asm("movl %%eax,%0":"=r"(reax)::);
    asm("movl %%ebx,%0":"=r"(rebx)::);
    asm("movl %%ecx,%0":"=r"(recx)::);
    asm("movl %%edx,%0":"=r"(redx)::);
    ireax = (int)reax;
    irebx = (int)rebx;
    irecx = (int)recx;
    iredx = (int)redx;
    printf("rdmsr(0x10):\n");
    printf("   eax = 0x%08x\n", ireax);
    printf("   ebx = 0x%08x\n", irebx);
    printf("   ecx = 0x%08x\n", irecx);
    printf("   edx = 0x%08x\n", iredx);

    return 0;
}
