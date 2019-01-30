/* dhcp.c - getlocalip */

/* Modifications - Stan Wileman
 *
 *	If 'network_status' is false (0), then skip DHCP query and
 *	report SYSERR from getlocalip.
 *
 *	Changed DHCP response timeout to 5000 msec instead of 2000 msec.
 *
 *	Added code to skip DHCP if we have a valid IP address and the
 *	lease is still valid.
 *
 *	Added code to get lease time, lease start time,
 *	RFC868 timeserver address, and nameserver (DNS) address from
 *	the data obtained using DHCP to the NetData structure.
 *
 *	Added code to set static values for many items in the NetData
 *	structure, as desired.
 */

#include <xinu.h>

extern uint32 network_status;

/*------------------------------------------------------------------------
 * dhcp_get_opt_val  -	Retrieve a pointer to the value for a specified
 *			  DHCP options key
 *------------------------------------------------------------------------
 */

char* 	dhcp_get_opt_val(
	  const struct dhcpmsg* dmsg, 	/* DHCP Message			*/
	  uint32 dmsg_size, 		/* Size of DHCP Message		*/
	  uint8 option_key		/* Option key to retrieve	*/
	)
{
	unsigned char* opt_tmp;
	unsigned char* eom;

	eom = (unsigned char*)dmsg + dmsg_size - 1;
	opt_tmp = (unsigned char*)dmsg->dc_opt;

	while(opt_tmp < eom) {

		/* If the option value matches return the value */

		if((*opt_tmp) == option_key) {

			/* Offset past the option value and the size 	*/

			return (char*)(opt_tmp+2);
		}
		opt_tmp++;	/* Move to length octet */
		opt_tmp += *(uint8*)opt_tmp + 1;
	}

	/* Option value not found */

	return NULL;
}

/*------------------------------------------------------------------------
 * dhcp_bld_bootp_msg  -  Set the common fields for all DHCP messages
 *------------------------------------------------------------------------
 */
void 	dhcp_bld_bootp_msg(struct dhcpmsg* dmsg)
{
	uint32	xid;			/* Xid used for the exchange	*/

	memcpy(&xid, NetData.ethucast, 4); /* Use 4 bytes from MAC as	*/
					   /*    unique XID		*/
	memset(dmsg, 0x00, sizeof(struct dhcpmsg));

	dmsg->dc_bop = 0x01;	     	/* Outgoing request		*/
	dmsg->dc_htype = 0x01;		/* Hardware type is Ethernet	*/
	dmsg->dc_hlen = 0x06;		/* Hardware address length	*/
	dmsg->dc_hops = 0x00;		/* Hop count			*/
	dmsg->dc_xid = htonl(xid);	/* Xid (unique ID)		*/
	dmsg->dc_secs = 0x0000;		/* Seconds			*/
	dmsg->dc_flags = 0x0000;	/* Flags			*/
	dmsg->dc_cip = 0x00000000;	/* Client IP address		*/
	dmsg->dc_yip = 0x00000000;	/* Your IP address		*/
	dmsg->dc_sip = 0x00000000;	/* Server IP address		*/
	dmsg->dc_gip = 0x00000000;	/* Gateway IP address		*/
	memset(&dmsg->dc_chaddr,'\0',16);/* Client hardware address	*/
	memcpy(&dmsg->dc_chaddr, NetData.ethucast, ETH_ADDR_LEN);
	memset(&dmsg->dc_bootp,'\0',192);/* Zero the bootp area		*/
	dmsg->dc_cookie = htonl(0x63825363); /* Magic cookie for DHCP	*/
}

/*------------------------------------------------------------------------
 * dhcp_bld_disc  -  handcraft a DHCP Discover message in dmsg
 *------------------------------------------------------------------------
 */
int32 	dhcp_bld_disc(struct dhcpmsg* dmsg)
{
	uint32  j = 0;

	dhcp_bld_bootp_msg(dmsg);

	dmsg->dc_opt[j++] = 0xff & 53;	/* DHCP message type option	*/
	dmsg->dc_opt[j++] = 0xff &  1;	/* Option length		*/
	dmsg->dc_opt[j++] = 0xff &  1;	/* DHCP Dicover message		*/
	dmsg->dc_opt[j++] = 0xff &  0;	/* Options padding		*/

	dmsg->dc_opt[j++] = 0xff & 55;	/* DHCP parameter request list	*/
	dmsg->dc_opt[j++] = 0xff &  2;	/* Option length		*/
	dmsg->dc_opt[j++] = 0xff &  1;	/* Request subnet mask 		*/
	dmsg->dc_opt[j++] = 0xff &  3;	/* Request default router addr  */

	return (uint32)((char *)&dmsg->dc_opt[j] - (char *)dmsg + 1);
}

/*------------------------------------------------------------------------
 * dhcp_bld_req - handcraft a DHCP request message in dmsg
 *------------------------------------------------------------------------
 */
int32 	dhcp_bld_req(
	  struct dhcpmsg* dmsg,		/* DHCP message to build	*/
	  const struct dhcpmsg* dmsg_offer, /* DHCP offer message	*/
	  uint32 dsmg_offer_size	/* Size of DHCP offer message	*/
	)
{
	uint32  j = 0;
	uint32* server_ip;        	/* Take the DHCP server IP addr	*/
					/*   from DHCP offer message	*/

	dhcp_bld_bootp_msg(dmsg);
	dmsg->dc_sip = dmsg_offer->dc_sip; /* Server IP address		*/

	dmsg->dc_opt[j++] = 0xff & 53;	/* DHCP message type option	*/
	dmsg->dc_opt[j++] = 0xff &  1;	/* Option length		*/
	dmsg->dc_opt[j++] = 0xff &  3;	/* DHCP Request message		*/
	dmsg->dc_opt[j++] = 0xff &  0;	/* Options padding		*/

	dmsg->dc_opt[j++] = 0xff & 50;	/* Requested IP			*/
	dmsg->dc_opt[j++] = 0xff &  4;	/* Option length		*/
	*((uint32*)&dmsg->dc_opt[j]) = dmsg_offer->dc_yip;
	j += 4;

	/* Retrieve the DHCP server IP from the DHCP options */
	server_ip = (uint32*)dhcp_get_opt_val(dmsg_offer,
				    dsmg_offer_size, DHCP_SERVER_ID);

	if(server_ip == 0) {
		kprintf("Unable to get server IP add. from DHCP Offer\n");
		return SYSERR;
	}

	dmsg->dc_opt[j++] = 0xff & 54;	/* Server IP			*/
	dmsg->dc_opt[j++] = 0xff &  4;	/* Option length		*/
	*((uint32*)&dmsg->dc_opt[j]) = *server_ip;
	j += 4;

	return (uint32)((char *)&dmsg->dc_opt[j] - (char *)dmsg + 1);
}

/*------------------------------------------------------------------------
 * getlocalip - use DHCP to obtain an IP address.
 *
 *		Any of the following can also be specified as a static
 *		address by defining it with a dotted address value.
 *		If DHCP also supplies an address, then it (the DHCP value)
 *		is superceded by the static value.
 *
 *	STATIC_IP	IP address (also sets lease time to 0xffffffff)
 *	STATIC_MASK	netmask
 *	STATIC_ROUTER	Gateway (to other than the local network)
 *	STATIC_DNS	DNS server address (only one allowed)
 *
 * Timeserver addresses are always statically defined now, and are
 * initialized by getlocalip. There may be as many as 3 (depending on
 * the number of entries defined in 'struct network'; unused entries
 * should be defined as "0.0.0.0" instead of a real dotted IP address string.
 *
 *	STATIC_TIME0	RFC 868 timeserver address 1
 *	STATIC_TIME1	RFC 868 timeserver address 2
 *	STATIC_TIME2	RFC 868 timeserver address 3
 *	
 *------------------------------------------------------------------------
 */

#define STATIC_TIME0 "216.229.0.179"		/* nist1-lnk.binary.net */
#define STATIC_TIME1 "24.56.178.140"		/* www.nist.gov */
#define STATIC_TIME2 "137.48.187.88"		/* cs2.ist.unomaha.edu */

uint32	getlocalip(void)
{
	int32	slot;			/* UDP slot to use		*/
	struct	dhcpmsg dmsg_snd;	/* Holds outgoing DHCP messages	*/
	struct	dhcpmsg dmsg_rvc;	/* Holds incoming DHCP messages	*/

	int32	i, j;			/* Retry counters		*/
	int32	len;			/* Length of data sent		*/
	int32	inlen;			/* Length of data received	*/
	char	*optptr;		/* Pointer to options area	*/
	char	*eop;			/* Address of end of packet	*/
	int32	msgtype;		/* Type of DCHP message		*/
	uint32	addrmask;		/* Address mask for network	*/
	uint32	routeraddr;		/* Default router address	*/
	uint32	leasetime;		/* lease time			*/
	uint32	timeserver;		/* RFC868 time server address	*/
	uint32	DNSserver;		/* RFC1035 DNS server address	*/
	uint32	tmp;			/* Used for byte conversion	*/
	uint32* tmp_server_ip;		/* Temporary DHCP server pointer*/

	if (!network_status)
		return SYSERR;

	/* Do we currently have an IP address, and	*/
	/* is the lease still valid? If so, then just	*/
	/* return the current IP address.		*/

	if (NetData.ipvalid == TRUE &&
		clktime < NetData.leasetime + NetData.leasestarttime) {
			return NetData.ipucast;
	}

	NetData.ipvalid = FALSE;	/* so we'll look at all UDP packets */

	slot = udp_register(0, UDP_DHCP_SPORT, UDP_DHCP_CPORT);
	if (slot == SYSERR) {
		kprintf("getlocalip: cannot register with UDP\n");
		return SYSERR;
	}

	len = dhcp_bld_disc(&dmsg_snd);
	if(len == SYSERR) {
		kprintf("getlocalip: Unable to build DHCP discover\n");
		return SYSERR;
	}

	for (i = 0; i < DHCP_RETRY; i++) {
		udp_sendto(slot, IP_BCAST, UDP_DHCP_SPORT,
						(char *)&dmsg_snd, len);

		/* Read 3 incoming DHCP messages and check for an offer	*/
		/* 	or wait for three timeout periods if no message */
		/* 	arrives.					*/

		for (j=0; j<3; j++) {
			inlen = udp_recv(slot, (char *)&dmsg_rvc,
					    sizeof(struct dhcpmsg),5000);
			if (inlen == TIMEOUT) {
				continue;
			} else if (inlen == SYSERR) {
				return SYSERR;
			}

			/* Check that incoming message is a valid	*/
			/*   response (ID matches our request)		*/

			if (dmsg_rvc.dc_xid != dmsg_snd.dc_xid) {
				continue;
			}

			eop = (char *)&dmsg_rvc + inlen - 1;
			optptr = (char *)&dmsg_rvc.dc_opt;
			msgtype = addrmask = routeraddr = 0;
			leasetime = 0;
			timeserver = 0;
			DNSserver = 0;

			while (optptr < eop) {

			    switch (*optptr) {
				case 53:	/* Message type */
					msgtype = 0xff & *(optptr+2);
				break;

				case 1:		/* Subnet mask */
					memcpy((void *)&tmp, optptr+2, 4);
					addrmask = ntohl(tmp);
				break;

				case 3:		/* Router address */
					memcpy((void *)&tmp, optptr+2, 4);
					routeraddr = ntohl(tmp);
					break;

				/* ADDED */
				case 4:		/* Time server address */
					memcpy((void *)&tmp, optptr+2, 4);
					timeserver = ntohl(tmp);
					break;

				/* ADDED */
				case 6:		/* DNS server address */
					memcpy((void *)&tmp, optptr+2, 4);
					DNSserver = ntohl(tmp);
					break;

				/* ADDED */
				case 51:	/* IP address lease time */
					memcpy((void *)&tmp, optptr+2, 4);
					leasetime = ntohl(tmp);
					break;
			    }
			    optptr++;	/* Move to length octet */
			    optptr += (0xff & *optptr) + 1;
			}

			if (msgtype == 0x02) {	/* Offer - send request	*/
				len = dhcp_bld_req(&dmsg_snd, &dmsg_rvc,
								inlen);
				if(len == SYSERR) {
					kprintf("getlocalip: %s\n",
					  "Unable to build DHCP request");
					return SYSERR;
				}
				udp_sendto(slot, IP_BCAST, UDP_DHCP_SPORT,
						(char *)&dmsg_snd, len);
				continue;

			} else if (dmsg_rvc.dc_opt[2] != 0x05) {
				/* If not an ack skip it */
				continue;
			}

			if (addrmask != 0) {
				NetData.ipmask = addrmask;
			}
#ifdef STATIC_MASK
			NetData.ipmask = dot2ip(STATIC_MASK);
#endif

			if (routeraddr != 0) {
				NetData.iprouter = routeraddr;
			}
#ifdef STATIC_ROUTER
			dot2ip(STATIC_ROUTER, &NetData.iprouter);
#endif


			NetData.ipucast = ntohl(dmsg_rvc.dc_yip);
#ifdef STATIC_IP
			dot2ip(STATIC_IP, &NetData.ipucast);
			leasetime = 0xffffffff;
#endif
			if (leasetime != 0) {
				NetData.leasetime = leasetime;
			}

			NetData.ipprefix = NetData.ipucast &
							 NetData.ipmask;
			NetData.ipbcast = NetData.ipprefix |
							~NetData.ipmask;
			NetData.ipvalid = TRUE;
			udp_release(slot);

			/* Retrieve the boot server IP */
			if(dot2ip((char*)dmsg_rvc.sname,
					    &NetData.bootserver) != OK) {

			  /* Could not retrieve the boot server from	*/
			  /*  the  BOOTP fields, so use the DHCP server	*/
			  /*  address					*/
			  tmp_server_ip = (uint32*)dhcp_get_opt_val(
					&dmsg_rvc, len, DHCP_SERVER_ID);
			  if(tmp_server_ip == 0) {
			    kprintf("Cannot retrieve boot server addr\n");
				return (uint32)SYSERR;
			  }
			NetData.bootserver = ntohl(*tmp_server_ip);
			}
			memcpy(NetData.bootfile, dmsg_rvc.bootfile,
					     sizeof(dmsg_rvc.bootfile));

			NetData.leasestarttime = clktime;

#ifdef STATIC_DNS
			dot2ip(STATIC_DNS, &DNSserver);
#endif

			/*-----------------------------*/
			/* Setup timeserver addresses. */
			/*-----------------------------*/
			dot2ip(STATIC_TIME0,&timeserver);
			NetData.tsaddr[0] = timeserver;

			dot2ip(STATIC_TIME1,&timeserver);
			NetData.tsaddr[1] = timeserver;

			dot2ip(STATIC_TIME2,&timeserver);
			NetData.tsaddr[2] = timeserver;

			NetData.nsaddr = DNSserver;

			/* XXX Shouldn't the UDP slot be released here? */

			return NetData.ipucast;
		}
	}

	kprintf("DHCP failed to get response\n");
	udp_release(slot);
	return (uint32)SYSERR;
}
