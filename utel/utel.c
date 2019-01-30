/*----------------------*/
/* utel client - utel.c */
/* usage:               */
/*     utel hostip      */
/*                      */
/* Stan Wileman         */
/*----------------------*/

/*-----------------------------------------------------------------*/
/* This is a simple "telnet-like" program that runs on a Linux     */
/* host and provides an interactive connection with a Xinu system. */
/* It uses two threads. One continually tries to read from the     */
/* network and write to the display. Another continually tries to  */
/* read from the keyboard and write to the network. The program    */
/* will terminate when it reads EOF from the keyboard.             */
/*                                                                 */
/* Compile the program like this:                                  */
/*	cc -o utel utel.c -lpthread                                */
/*                                                                 */
/* To use the program, make certain the Xinu system is running and */
/* the network is operational. Let's assume the Xinu IP address is */
/* 1.2.3.4 (you can find its IP address with the ipaddr command).  */
/* Start the 'ss' program on Xinu. It will block, waiting for a    */
/* connection from this program. On the Linux system with utel,    */
/* use this command: ./utel 1.2.3.4                                */
/*                                                                 */
/* This program only uses UDP, since the distributed version of    */
/* Xinu for the Galileo doesn't have Xinu's TCP code provided.     */
/*-----------------------------------------------------------------*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>

#define UELPORT 42346	/* utel UDP port number on server */

#define MAXBUF 10*1024

static int sk;				/* UDP socket */
static struct sockaddr_in server;	/* server address */

/*---------------------------------------------------------------*/
/* Thread t1: read lines from the keyboard, write to the remote. */
/* At end of file, close the "connection" and terminate.         */
/*---------------------------------------------------------------*/
void *t1code(void *arg)
{
  int n;				/* # of bytes read */
  int n_sent;				/* # of bytes written */
  char buff[1000];			/* buffer */

  for(;;) {
    n = read(0,buff,1000);		/* read a line (a kludge to be sure) */
    if (n == 0)
      break;
    if (n < 0) {
      perror("t1 read");
      return NULL;			/* terminate the thread */
    }
    n_sent = sendto(sk, buff, n, 0,
      (struct sockaddr *) &server,sizeof(server));
    if (n_sent != n) {
      printf("t1: rror sending buf.\n");
      perror("sendto");
      return NULL;
    }
  }

  /*-------------*/
  /* Disconnect. */
  /*-------------*/
  n_sent = sendto(sk, buff, 0, 0,
    (struct sockaddr *) &server,sizeof(server));
  return NULL;
}

/*-------------------------------------------------*/
/* Read from the UDP socket, write to the display. */
/*-------------------------------------------------*/
void *t2code(void *arg)
{
  int n;				/* # of bytes read */
  char buff[1000];			/* buffer */

  for(;;) {
    n = recvfrom(sk, buff, 1000, 0, NULL, NULL);
    if (n < 0) {
	perror("t2: recvfrom");
	return NULL;
    }
    write(1,buff,n);
  }
}

int main(int argc, char *argv[])
{
  struct hostent *hp;
  int n_sent;
  int n_read;
  int i;
  int n;			/* # of bytes read */
  int c;			/* result of getchar() */
  int buf_len;
  char buf[1];			/* very short buffer */
  int status;			/* status from pthread_create */
  void *result;			/* thread return value (ignored) */
  pthread_t t1, t2;		/* pthread identifiers */

  /*----------------------------------------------------------*/
  /* Make sure we have the right number of command line args. */
  /*----------------------------------------------------------*/
  if (argc != 2) {
    fprintf(stderr,"Usage: utel hostIP\n");
    exit(0);
  }

  /*-----------------------------------------------------------------*/
  /* create a socket: IP protocol family (PF_INET), UDP (SOCK_DGRAM) */
  /*-----------------------------------------------------------------*/
  if ((sk = socket( PF_INET, SOCK_DGRAM, 0 )) < 0) {
      fprintf(stderr,"Problem creating socket\n");
      exit(1);
  }

  /*--------------------------------------------------------------*/
  /* Using UDP we don't need to call bind unless we care what our */
  /* local port number is - most clients don't care.              */
  /*--------------------------------------------------------------*/

  /*------------------------------------------------------------*/
  /* Create a sockaddr that will be used to contact the server. */
  /* Fill in an address structure that will be used to specify  */
  /* the address of the server to which we want to connect.     */
  /*                                                            */
  /* address family is IP  (AF_INET)                            */
  /*                                                            */
  /* server IP address is found by calling gethostbyname with   */
  /* the name of the server (entered on the command line).      */
  /*                                                            */
  /* Server port number is UELPORT.                             */
  /*------------------------------------------------------------*/
  server.sin_family = AF_INET;
  if ((hp = gethostbyname(argv[1]))==0) {
    fprintf(stderr,"Invalid or unknown host\n");
    exit(1);
  }

  /*--------------------------------------------------------------------*/
  /* copy the IP address into the sockaddr; It's in network byte order. */
  /*--------------------------------------------------------------------*/
  memcpy( &server.sin_addr.s_addr, hp->h_addr, hp->h_length);

  /*-----------------------------------------------------------*/
  /* Establish the server port number; use network byte order. */
  /*-----------------------------------------------------------*/
  server.sin_port = htons(UELPORT);

  /*--------------------------------------------*/
  /* Send a 0-length packet to the utel server. */
  /*--------------------------------------------*/
  buf_len = 0;
  n_sent = sendto(sk, buf, buf_len, 0,
                  (struct sockaddr *) &server,sizeof(server));

  if (n_sent < 0) {
    perror("sendto");
    exit(1);
  }

  /*--------------------------------------------*/
  /* Start two threads to do the communication. */
  /*--------------------------------------------*/
  status = pthread_create(&t1, NULL, t1code, (void *)NULL);
  if (status != 0) {
    perror("t1 creation");
    exit(1);
  }

  status = pthread_create(&t2, NULL, t2code, (void *)NULL);
  if (status != 0) {
    perror("t2 creation");
    exit(1);
  }

  /*----------------------------------------------------*/
  /* Wait for t1 to finish. When it does, terminate t2. */
  /*----------------------------------------------------*/
  pthread_join(t1, &result);

  /* XXX Worry about t2 failing. */
  /* XXX How do we force t2 to die (or any thread)? -- pthread_cancel. */
  return 0;
}
