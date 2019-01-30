/* xsh_tftp.c - xsh_tftp */

#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*------------------------------------------------------------------------
 * xsh_tftp - shell command for trivial FTP
 *------------------------------------------------------------------------
 * Usage:
 *    tftp host
 *    tftp --help
 *
 * Once tftp is running, the following commands may be used
 *    get remotefilename [localfilename]
 *    put localfilename [remotefilename]
 *    quit (or end of file)
 *
 */

#define MAXPACKET 1000			/* maximum packet size (in or out) */
#define MAXLINE 80			/* maximum command line length */
#define TOTIME 5000			/* timeout, in milliseconds */
#define MAXTOUT 4			/* maximum # of timeouts allowed */
static char line[MAXLINE+1];		/* input command line */
static int istrunc;			/* non-zero if input line truncated */
static char *rnamep;			/* pointer to remote filename */
static char *lnamep;			/* pointer to local filename */
static uint32 serverip;			/* IP address of server */
static uint16 srport;			/* server's port number */
static uint16 expblkno;			/* expected block number */
static uint16 tocount;			/* timeout count */

enum tftp_op {				/* TFTP operation codes */
    op_rrq = 1,				    /* read request */
    op_wrq = 2,				    /* write request */
    op_data = 3,			    /* data */
    op_ack = 4,				    /* acknowledgement */
    op_error = 5			    /* error */
};

char opkt[MAXPACKET];			/* outward bound packet */
char ipkt[MAXPACKET];			/* inward bound packet */

static int vopt;			/* verbose option */

/*--------------------------------------------------------------------*/
/* Display a hexadecimal/character dump of 'len' bytes starting at p. */
/*--------------------------------------------------------------------*/
static void dump(char *p, int32 len)
{
    char *xp;			/* actual display address */
    char *ep;			/* first address not displayed */
    char *aep;			/* last byte to be displayed */
    char *cp;			/* current byte being displayed */
    int32 i;

    if (len <= 0)		/* obvious do-nothing case */
	return;

    xp = (char *)((uint32)p & ~15);	/* round start down to multiple of 16 */
    ep = (char *)((uint32)p+len+15);	/* round end up to multiple of 16 */
    aep = p + len - 1;

    while (xp < ep) {

	/*-----------------*/
	/* Display address */
	/*-----------------*/
	printf("%08x: ", xp);

	/*------------------*/
	/* Dump data in hex */
	/*------------------*/
	for(i=0;i<16;i++) {
	    cp = xp + i;
	    if (cp < p || cp > aep)
		printf("   ");
	    else
		printf("%02x ", *cp);
	}

	/*--------------------*/
	/* Dump data in ASCII */
	/*--------------------*/
	printf(" ");
	for(i=0;i<16;i++) {
	    cp = xp + i;
	    if (cp < p || cp > aep)
		printf(" ");
	    else if (*cp >= ' ' && *cp <= '~')
		printf("%c", *cp);
	    else
		printf(".");
	}
	printf("\n");

	xp += 16;
    }
}

/*--------------------------------------------------------------*/
/* Get the next line from 'dev' into 'line' (max len = MAXLINE) */
/* Don't include the terminating end of line character, but do  */
/* end with a null byte.                                        */
/* Return the number of bytes in the line, excluding the end of */
/* line character. Return EOF at end of file.                   */
/*--------------------------------------------------------------*/
static int getline(int dev)
{
    char *p = line;
    int n, len = 0;
    char c;

    istrunc = 0;
    for(;;) {
        n = read(dev,&c,1);
        if (n == SYSERR) {
            printf("Error reading device %d in getline.\n", dev);
            kill(getpid());
        }
        if (n == EOF)
            return EOF;
        if (n == 0) {
            if(isatty(dev))	/* 0 == EOF only if we're not on a terminal */
		break;		/* Otherwise it just means end of line */
            return EOF;
        }
        if (c == '\n') break;
        if (len < MAXLINE-1) {
            *p++ = c;
            len++;
        } else istrunc = 1;
    }
    *p = '\0';
    return len;
}

/*---------------------------------------------------------------*/
/* Get one or two filenames from the command line, and check for */
/* trailing trash. Return 1 if successful, 0 on failure.         */
/* If dir is 0, names expected are in the order remote, local.   */
/* Otherwise they should be in the order local, remote.          */
/* If only one name is given, it is used for local and remote.   */
/*---------------------------------------------------------------*/
static int getfnames(int dir)
{
    char *f1, *f2;

    f1 = f2 = NULL;
    f1 = strtok(NULL," \t");	/* get first filename */
    if (f1 == NULL) {
	printf("--no filename(s)\n");
	return 0;
    }

    f2 = strtok(NULL," \t");	/* get optional second filename */
    if (f2 == NULL)		/* if only one name, use same for both */
	f2 = f1;
    else if (strtok(NULL," \t") != NULL) {	/* check for trailing trash */
	printf("--too many arguments\n");
	return 0;
    }

    if (dir) {
	lnamep = f1;
	rnamep = f2;
    } else {
	rnamep = f1;
	lnamep = f2;
    }
    return 1;
}

/*-------------------------------------------------------------------------*/
/* Get a local TID (UDP port number) that is unique among the last NUNTID  */
/* TIDs used in this invocation of the program. To do that, we maintain a  */
/* static record of the last NUNTID TIDs used. Each time a new one is used */
/* the oldest one is replaced (assuming we've saved NUNTID previous TIDs). */
/* Then create a socket using that TID. Return OK on success, and save the */
/* socket at the address given by psock. On failure return SYSERR.         */
/*-------------------------------------------------------------------------*/
#define NUNTID 16			/* size of unique TID cache */

static int32 getlocalport(int32 *psock)
{
    static uint16 tidc[NUNTID];		/* cache of most recently used TIDs */
    static uint16 ntid=0;		/* # of TIDs in the TID cache */
    static uint16 lasttidi=0;		/* last used TID's index in cache */
    uint32 now;				/* time (from getutime) */
    uint16 tid;				/* tid being considered */
    uint16 i, j;			/* index */
    bool8 found;			/* was TID candidate already used? */
    int32 sock;				/* local socket number */

    if (getutime(&now) == SYSERR)	/* use time as a random number seed */
	now = getpid();			/* if not available, use our pid */
    srand((uint32)now);			/* seed psuedorandom number generator */

    for(;;) {
	now = rand();			/* get a random number */
	tid = 50000 + (now & 0xfff);	/* generate a TID candidate */
	found = FALSE;			/* assume TID is unique */
	for(i=0;i<ntid;i++) {		/* check last ntid cache entries */
	    j = lasttidi - i;
	    if (j < 0)
		j += NUNTID;
	    if (tidc[j] == tid) {
		found = TRUE;
		break;
	    }
	}
	if (found == TRUE)		/* if tid used recently, try again */
	    continue;			/* we assume NUNTID < period of rand */

	if (vopt)
	    printf("[TFTP] trying to use local TID %d\n", tid);

	/*-------------------------------------------*/
	/* Try to register the UDP socket we'll use. */
	/*-------------------------------------------*/
	sock = udp_register(serverip, 0, tid);
	if (sock == SYSERR)		/* if this tid won't work, try again */
	    continue;			/* XXX do we need a retry counter? */

	/*---------------------------*/
	/* Save the TID we're using. */
	/*---------------------------*/
	if (ntid < NUNTID) {		/* if the cache isn't full yet */
	    tidc[ntid] = tid;
	    lasttidi = ntid;
	    ntid++;
	} else {
	    lasttidi = (lasttidi + 1) % NUNTID;
	    tidc[lasttidi] = tid;
	}

	*psock = sock;
	return OK;
    }
}

/*------------------------------------------------------*/
/* Prepare a read or write request packet in opkt.      */
/* The packet format is as follows:                     */
/*                                                      */
/*   2 bytes     string    1 byte     string   1 byte   */
/*  +--------+------------+------+------------+------+  */
/*  | Opcode |  Filename  |   0  |    Mode    |   0  |  */
/*  +--------+------------+------+------------+------+  */
/*                                                      */
/* The length of the packet is returned.                */
/*------------------------------------------------------*/
static uint16 prep_req_pkt(enum tftp_op opn)
{
    char *p = opkt;

    *(uint16 *)opkt = htons(opn);	/* fill in the operation code */
    strcpy(p+2,rnamep);			/* fill in remote file name */
    p += strlen(rnamep) + 3;		/* adjust p */
    strcpy(p,"octet");			/* fill in transfer mode */
    p += 6;				/* adjust p */

    if (vopt) {
	int32 len = (int32)(p - opkt);

	printf("Request packet:\n");
	dump(opkt,len);
    }

    return (uint16)(p - opkt);
}

static void do_get(void)
{
    did32 ofd;				/* local (output) file descriptor */
    int32 sock;				/* local socket */
    uint16 pklen;			/* packet length */
    int32 ret;				/* return from udp_sendto */

    if (!getfnames(0))
	return;

    if (vopt)
	printf("   copying remote file %s to local file %s\n", rnamep, lnamep);

    /*--------------------------------*/
    /* Open local file in write mode. */
    /*--------------------------------*/
    ofd = open(NAMESPACE,lnamep,"w");
    if (ofd == SYSERR) {
	printf("Unable to open local file %s.\n", lnamep);
	return;
    }
    control(ofd, F_CTL_TRUNC, 0, 0);	/* delete any content in local file */

    /*------------------------------------*/
    /* Get a local TID (UDP port number). */
    /*------------------------------------*/
    if (getlocalport(&sock) == SYSERR) {
	printf("Unable to obtain an acceptable local UDP socket.\n");
	close(ofd);
	remove(lnamep,0);
	return;
    }

    /*-------------------------------------------*/
    /* Prepare and send the read request packet. */
    /*-------------------------------------------*/
    srport = TFTP_PORT;			/* initial port number */
    pklen = prep_req_pkt(op_rrq);

    if (vopt) {
	printf("Sending read request packet to server\n");
	dump(opkt,pklen);
    }
    ret = udp_sendto(sock, serverip, srport, opkt, pklen);
    if (ret == SYSERR) {
	printf("-->error sending read request to server\n");
	close(ofd);
	remove(lnamep,0);
	udp_release(sock);
	return;
    }
    expblkno = 1;
    tocount = 0;

    for(;;) {
	int32 n;			/* # of bytes in received packet */
	uint32 temp;			/* sender's IP address */

	if (vopt)
	    printf("trying receive from port %d\n", srport);
	n = udp_recvaddr(sock, &temp, &srport, ipkt, MAXPACKET, TOTIME);

	if (n == SYSERR) {
	    printf("-->error receiving packet from server\n");
	    close(ofd);
	    remove(lnamep,0);
	    udp_release(sock);
	    return;
	}

	if (n == TIMEOUT) {
	    tocount++;
	    if (vopt) printf("-->timeout %d\n", tocount);
	    if (tocount >= MAXTOUT) {
		printf("-->too many timeouts waiting on server\n");
		close(ofd);
		remove(lnamep,0);
		udp_release(sock);
		return;
	    }
	    ret = udp_sendto(sock, serverip, srport, opkt, pklen);
	    if (ret == SYSERR) {
		printf("-->error sending read request to server\n");
		close(ofd);
		remove(lnamep,0);
		udp_release(sock);
		return;
	    }
	    continue;
	}

	if (vopt) {
	    printf("Dump of (parts of) received packet from server\n");
	    printf("Packet size = %d bytes\n", n);
	    dump(ipkt,n > 16 ? 16 : n);
	}

	if (n < 4) {			/* packet is too small */
	    if (vopt) printf("-->small packet (n = %d)\n", n);
	    continue;
	}

	if (ntohs(*(uint16 *)ipkt) == op_error) {
	    uint16 ecode = ntohs(*(uint16 *)(ipkt+2));
	    printf("-->server sent error code %d\n", ecode);
	    printf("-->%s\n", ipkt+4);
	    close(ofd);
	    remove(lnamep,0);
	    udp_release(sock);
	    return;
	}

	/*--------------------------------------------------*/
	/* Check for DATA opcode and correct block number.  */
	/* If correct, then save any data, ack, and repeat. */
	/*--------------------------------------------------*/
	if (ntohs(*(uint16 *)ipkt) == op_data &&
	    ntohs(*(uint16 *)(ipkt+2)) == expblkno) {
	    if (vopt) printf("got DATA opcode and right block number\n");
	    if (n > 4) {
		if (vopt) {
		    printf("..writing %d bytes to local file.\n", n-4);
		    dump(ipkt+4,n-4);
		}
		/* XXX - need to check result */
		write(ofd,ipkt+4,n-4);
	    }
	    *(uint16 *)opkt = htons(op_ack);
	    *(uint16 *)(opkt+2) = htons(expblkno);
	    if (vopt) {
		printf("Sending ACK packet to server\n");
		dump(opkt,4);
	    }
	    if (vopt) {
		printf("Sending ACK packet to port %d\n", srport);
		dump(opkt,4);
	    }
	    ret = udp_sendto(sock, serverip, srport, opkt, 4);
	    if (vopt) printf("Send ACK status = %d\n", ret);
	    /* We should really check the result, yes? */
	    if (n-4 < 512) {		/* end of file? */
		if (vopt) printf("..end of file\n");
		close(ofd);
		udp_release(sock);
		return;
	    }
	    expblkno++;
	    continue;
	}

	/* Something else happened, which we'll ignore */
	printf("Unknown event; ignored\n");
    }
    return;	/* not reached */
}

static void do_put(void)
{
    did32 ifd;				/* local (input) file descriptor */
    int32 sock;				/* local socket */
    uint16 pklen;			/* packet length */
    uint16 lpkdsz;			/* # data bytes in last packet */
    int32 ret;				/* return from udp_sendto */

    if (!getfnames(1))
	return;

    if (vopt)
	printf("   copying local file %s to remote file %s\n", lnamep, rnamep);

    /*--------------------------------*/
    /* Open local file in read  mode. */
    /*--------------------------------*/
    ifd = open(NAMESPACE,lnamep,"ro");
    if (ifd == SYSERR) {
	printf("Unable to open local file %s.\n", lnamep);
	return;
    }

    /*------------------------------------*/
    /* Get a local TID (UDP port number). */
    /*------------------------------------*/
    if (getlocalport(&sock) == SYSERR) {
	printf("Unable to obtain an acceptable local UDP socket.\n");
	close(ifd);
	remove(lnamep,0);
	return;
    }

    /*--------------------------------------------*/
    /* Prepare and send the write request packet. */
    /*--------------------------------------------*/
    srport = TFTP_PORT;			/* initial port number */
    pklen = prep_req_pkt(op_wrq);

    if (vopt) {
	printf("Sending write request packet to server\n");
	dump(opkt,pklen);
    }
    ret = udp_sendto(sock, serverip, srport, opkt, pklen);
    if (ret == SYSERR) {
	printf("-->error sending write request to server\n");
	close(ifd);
	udp_release(sock);
	return;
    }
    expblkno = 0;
    lpkdsz = 512;
    tocount = 0;

    for(;;) {
	int32 n;			/* # of bytes in received packet */
	uint32 temp;			/* sender's IP address */

	if (vopt)
	    printf("trying receive from port %d\n", srport);
	n = udp_recvaddr(sock, &temp, &srport, ipkt, MAXPACKET, TOTIME);

	if (n == SYSERR) {
	    printf("-->error receiving packet from server\n");
	    close(ifd);
	    udp_release(sock);
	    return;
	}

	if (n == TIMEOUT) {
	    tocount++;
	    if (vopt) printf("-->timeout %d\n", tocount);
	    if (tocount >= MAXTOUT) {
		printf("-->too many timeouts waiting on server\n");
		close(ifd);
		udp_release(sock);
		return;
	    }
	    /*------------------------*/
	    /* Resend the DATA packet */
	    /*------------------------*/
	    ret = udp_sendto(sock, serverip, srport, opkt, pklen);
	    if (ret == SYSERR) {
		printf("-->error sending packet to server\n");
		close(ifd);
		udp_release(sock);
		return;
	    }
	    continue;
	}

	if (vopt) {
	    printf("Dump of (parts of) received packet from server\n");
	    printf("Packet size = %d bytes\n", n);
	    dump(ipkt,n > 16 ? 16 : n);
	}

	if (n < 4) {			/* packet is too small */
	    if (vopt) printf("-->small packet (n = %d)\n", n);
	    continue;
	}

	if (ntohs(*(uint16 *)ipkt) == op_error) {
	    uint16 ecode = ntohs(*(uint16 *)(ipkt+2));
	    printf("-->server sent error code %d\n", ecode);
	    printf("-->%s\n", ipkt+4);
	    close(ifd);
	    udp_release(sock);
	    return;
	}

	/*------------------------------------------------*/
	/* Check for ACK opcode and correct block number. */
	/* If correct, then prep and send a data packet   */
	/* (if more data is left to send).                */
	/*------------------------------------------------*/
	if (ntohs(*(uint16 *)ipkt) == op_ack && n == 4 && 
	    ntohs(*(uint16 *)(ipkt+2)) == expblkno) {
	    if (vopt) printf("got ACK with right block number\n");
	    if (lpkdsz < 512) {
		close(ifd);
		udp_release(sock);
		return;
	    }

	    n = read(ifd,opkt+4,512);
	    if (n == SYSERR) {
		printf("Error reading local file.\n");
		close(ifd);
		udp_release(sock);
		return;
	    }
	    lpkdsz = n;

	    *(uint16 *)opkt = htons(op_data);
	    expblkno++;
	    *(uint16 *)(opkt+2) = htons(expblkno);
	    if (vopt) {
		printf("Sending DATA packet to server\n");
	    }
	    pklen = n + 4;
	    ret = udp_sendto(sock, serverip, srport, opkt, pklen);
	    if (vopt) printf("Send DATA status = %d\n", ret);
	    if (ret == SYSERR) {
		printf("Error sending to server.\n");
		close(ifd);
		udp_release(sock);
		return;
	    }
	    tocount = 0;
	    continue;
	}

	/* Something else happened, which we'll ignore */
	printf("Unknown event; ignored\n");
    }
    return;	/* not reached */
}

shellcmd xsh_tftp(int nargs, char *args[])
{
    int len;			/* length of command line */
#if 0
    uint32 localip;		/* local IP address to use */
#endif
    char *cmdp;			/* pointer to command keyword */

#if 0
    int	i;			/* index into buffer */
    int	retval;			/* return value */
    char inbuf[1500];		/* buffer for incoming reply */
    int32 slot;			/* UDP slot to use */
    int32 msglen;		/* length of outgoing message */
    uint16 echoport = 7;	/* port number for UDP echo */
    uint16 locport = 52743;	/* local port to use */
    int32 retries = 3;		/* number of retries */
    int32 delay	= 2000;		/* reception delay in ms */
#endif

    /*-----------------------------------------------------------*/
    /* For argument '--help', emit help about the 'tftp' command */
    /*-----------------------------------------------------------*/
    if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
	printf("Use:\n    tftp  host\n    or\n    tftp --help\n");
	return 0;
    }

    /*------------------------------*/
    /* Recognize any other options. */
    /*------------------------------*/
    vopt = 0;
    while (nargs > 1 && args[1][0] == '-') {
	if (!strcmp(args[1],"-v")) {
	    vopt = 1;
	    nargs--;
	    args++;
	    continue;
	}
	printf("Unrecognized option %s\n", args[1]);
	return 0;
    }

    /*-------------------------------------*/
    /* Check for valid IP address argument */
    /*-------------------------------------*/
    if (nargs != 2) {
	printf("tftp: invalid argument\n");
	printf("Try 'tftp --help' for more information\n");
	return 0;
    }

    if (dot2ip(args[1], &serverip) == SYSERR) {
	printf("tftp: invalid IP address argument\n");
	return 0;
    }

    if (vopt) {
	printf("serverip = 0x%08x\n", serverip);
	printf("should be  0x%08x\n",
	    (192 << 24) | (168 << 16) | (1 << 8) | 235);
    }

#if 0
    localip = getlocalip();
    if (localip == SYSERR) {
	printf("tftp: could not obtain a local IP address\n");
	return 0;
    }
#endif

    /*----------------------------*/
    /* Read and process commands. */
    /*----------------------------*/
    for(;;) {
	if (isatty(stdin))
	    printf("tftp> ");			/* prompt for a command */
	len = getline(stdin);
	if (len == EOF)				/* end of file? */
	    break;
	if (len == 0)				/* empty line? */
	    continue;
	cmdp = strtok(line," \t");		/* find command */
	if (cmdp == NULL)			    /* empty line */
	    continue;
	if (!strcmp(cmdp,"quit"))
	    break;
	if (!strcmp(cmdp,"get")) {
	    do_get();
	    continue;
	}
	if (!strcmp(cmdp,"put")) {
	    do_put();
	    continue;
	}
	printf("--unknown command\n");
    }
    printf("\n");
    return 0;
}

/***************************************************************************
Pseudocode for XINU TFTP Program
================================

Get file "remote" into file "local"
----------------------------------
0. Open file "local" in "write" mode. If this fails, quit with an appropriate
   error message.

1. Choose a local TID (port number). This should be unique, and if we are
   doing multiple file transfers in the same session, it should be different
   from a previous TID. This TID must be no larger than 65535 since it must
   fit in a 2-byte field. The existing XINU code to use TFTP for booting uses
   (the time returned by getutime & 0xfff) + 50000 as the local port number
   (or TID). This will be in the range 50000..54095 (since 0xfff = 4095).

2. Register a UDP socket with the remote server's IP, 0 (our IP), and the
   local port number (TID). If that fails, choose a different TID (step 1).
   Repeat until success, or until a specified number of retry attempts have
   been tried.

3. Send the request message to the server's port 69:
	RRQ	"remote"		octet
   If an error is indicated on sending this packet, display an error message
   and fail the reception.
   Set the expected block number (expblkno) to 1.

4. Set the timeout counter to 0.
   Wait for a response to the RRQ packet. It should be:
	OP=3	BLK#=(expblkno)	DATA (at most 512 bytes)
   Also extract the sender's port number, since it will likely not be 69.
   This is the number we'll use in all future communication.

   Here are the other responses that may occur, and what we're to do in
   each case:

   4a. Packet too short to be legal.
	Ignore the packet, and wait for another response. But do not
	reset the timeout counter.

   4b. Bad port number.
	Ignore the packet, and wait for another response. But do not
	reset the timeout counter.

   4a. OP=ERR (some error)
	Display the error code and the message and fail the reception.

   4b. Timeout
	Resend the RRQ packet. Increment the timeout counter.
	 If too many timeouts have occurred, display an appropriate error
	message and fail the reception.

   4c. OP=ACK
	Ignore the packet, and wait for another response. But do not
	reset the timeout counter.

   4d. OP=DATA(3), BLK# != (expblkno).
	Ignore the packet, and wait for another response. But do not
	reset the timeout counter.

5. Write any received data (1..512 bytes) to the local file.
   Of course don't write anything if the data size in the packet was 0.

6. Send a packet to the remote server:
	ACK	BLK# (from the packet just read)

7. If the packet contained less than 512 bytes of data,
   then close the file, since we're done. Also release the socket reservation.

8. Increment the expected block number (expblkno) and return to step 4.


Send file "local" to file "remote"
----------------------------------
0. Open file "local" in "read" mode. If this fails, quit with an appropriate
   error message.

1. Choose a local TID (port number). This should be unique, and if we are
   doing multiple file transfers in the same session, it should be different
   from a previous TID. This TID must be no larger than 65535 since it must
   fit in a 2-byte field. The existing XINU code to use TFTP for booting uses
   (the time returned by getutime & 0xfff) + 50000 as the local port number
   (or TID). This will be in the range 50000..54095 (since 0xfff = 4095).

2. Register a UDP socket with the remote server's IP, 0 (our IP), and the
   local port number (TID). If that fails, choose a different TID (step 1).
   Repeat until success, or until a specified number of retry attempts have
   been tried.

3. Send the write request message to the server's port 69:
	WRQ	"remote"		octet
   If an error is indicated on sending this packet, display an error message
   and fail the reception.
   Set the expected block number (expblkno) to 0.
   Set lpkdsz to 512 (last packet data size).
   Clear the saved server's "real" port number.

4. Set the timeout counter to 0.
   Wait for a response to the WRQ packet. It should be:
	OP=4	BLK#=(expblkno)
   If we haven't yet saved the server's "real" port number, do it now.

   Here are the other responses that may occur, and what we're to do in
   each case:

   4a. Packet too short.
	Ignore, continue waiting. (Do not increment timeout counter.)

   4b. Bad sender port number in packet (if expected port number saved).
	Ignore, continue waiting. (Do not increment timeout counter.)

   4c. OP=ERR (some error)
	Display the error code and the message and fail the reception.

   4d. Timeout
	Resend the WRQ packet. Increment the timeout counter.
	 If too many timeouts have occurred, display an appropriate error
	message and fail the reception.

   4e. OP=DATA
	Ignore the packet, and wait for another response. But do not
	reset the timeout counter.

   4f. OP=ACK, BLK# != (expblkno)
	Ignore the packet, and wait for another response. But do not
	reset the timeout counter.

5. If lpkdsz (last packet data size) was less than 512, we're done.

6. Read the next 512 (or fewer) bytes from the file.
   Set lpkdsz to the number of bytes (0..512).
   Prepare the appropriate data packet, even one with zero bytes.

7. Send the data packet to the remote server:
	DATA	BLK#(expblkno)	     data from file (if any)

8. Increment the expected block number (expblkno) and return to step 4.

***************************************************************************/
