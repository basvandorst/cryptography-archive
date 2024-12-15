/*
 * The author of this software is John Ioannidis, ji@cs.columbia.edu.
 * Copyright (C) 1993, by John Ioannidis.
 *	
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTY. IN PARTICULAR, THE AUTHOR DOES MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE
 * MERCHANTABILITY OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR
 * PURPOSE.
 */

/*
 * swIPe debugging tool (yeah, right)
 */

#define DEBUGSW
#define SWIPEDEBUG



#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <net/af.h>
#include <net/netisr.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>

#include <netinet/ip_swipe.h>
#include <netinet/if_sw.h>

#include <stdio.h>
#include <ctype.h>

#include <stdio.h>

#include <kvm.h>
#include <fcntl.h>

main(argc, argv)
int argc;
char **argv;
{
	int sd, cnt, i, j, c, auth=0, encr=0;
	struct swipevars swv;
	struct swkey_ent *se;
	struct sw_ipol *si;
	struct sw_opol *so;
	kvm_t *kd;
	struct mbuf mb;
	struct mbuf *m0, *m1;
	caddr_t base;
	char lin[80];
	
	
#ifdef nodef
	sd = socket(AF_INET, SOCK_RAW, 0);
	if (sd < 0)
	  perror("socket"), exit();
#endif
	sd = open("/dev/sw", 2);
	if (sd < 0)
	  perror("opening sw"), exit();
	

/*******8
	while (argc-- && (**++argv == '-'))
	{
		switch (argv[0][1])
		{
		      case 'a':
			auth++;
			break;
			
		      case 'e':
			encr++;
			break;
			
		      default:
			fprintf(stderr, "yo mama!\n");
		}
	}
	
	if ((argc != 4) && (argc != 6))
	  fprintf(stderr, "usage: sswo [-a] [-e] snd-key rcv-key isrc idst [osrc odst]\n");

**********/
	
		
	if (ioctl(sd, SWIPGSWVARS, &swv) < 0)
	  perror("SWIPGSWVARS");

	printf("skhashsiz: %d\n", swv.skhashsiz);
	printf("keysndp: 0x%08x\n", swv.keysndp);
	printf("keyrcvp: 0x%08x\n", swv.keyrcvp);
	printf("ipolicyp: 0x%08x\n", swv.ipolicyp);
	printf("opolicyp: 0x%08x\n", swv.opolicyp);
	printf("debugsw: 0x%08x\n", swv.debugsw);
	printf("swipedebug: 0x%08x\n", swv.swipedebug);

	kd = kvm_open(NULL, NULL, NULL, O_RDWR, "spr");

	printf("\nSEND KEYS:\n");

	base = (caddr_t)swv.keysndp;
	for (i=0; i<swv.skhashsiz; i++)
	{
		kvm_read(kd, base, &m0, sizeof(struct mbuf *));
		while (m0)
		{
			kvm_read(kd, (caddr_t)m0, &mb, sizeof(struct mbuf));
			se = mtod(&mb, struct swkey_ent *);
			printf("\n  %s->",
			       inet_ntoa(se->se_osrc));
			printf("%s, ",
			       inet_ntoa(se->se_odst));
			printf("seqnum: %d\n", se->se_seqnum);
			sprintf(lin,
				"authkey: len=%d, algo=%d", 
				se->se_authkey.sk_keylen,
				se->se_authkey.sk_algo);
			xdump(se->se_authkey.sk_data,
			      se->se_authkey.sk_keylen,
			      lin);
			sprintf(lin,
				"cryptkey: len=%d, algo=%d", 
				se->se_cryptkey.sk_keylen,
				se->se_cryptkey.sk_algo);
			xdump(se->se_cryptkey.sk_data,
			      se->se_cryptkey.sk_keylen,
			      lin);
			m0=mb.m_next;
		}
		base += (sizeof (struct mbuf *));
	}

	printf("\nRECEIVE KEYS:\n");

	base = (caddr_t)swv.keyrcvp;
	for (i=0; i<swv.skhashsiz; i++)
	{
		kvm_read(kd, base, &m0, sizeof(struct mbuf *));
		while (m0)
		{
			kvm_read(kd, (caddr_t)m0, &mb, sizeof(struct mbuf));
			se = mtod(&mb, struct swkey_ent *);
			printf("  %s->",
			       inet_ntoa(se->se_osrc));
			printf("%s, ",
			       inet_ntoa(se->se_odst));
			printf("\nseqnum: %d\n", se->se_seqnum);
			sprintf(lin,
				"authkey: len=%d, algo=%d", 
				se->se_authkey.sk_keylen,
				se->se_authkey.sk_algo);
			xdump(se->se_authkey.sk_data,
			      se->se_authkey.sk_keylen,
			      lin);
			sprintf(lin,
				"cryptkey: len=%d, algo=%d", 
				se->se_cryptkey.sk_keylen,
				se->se_cryptkey.sk_algo);
			xdump(se->se_cryptkey.sk_data,
			      se->se_cryptkey.sk_keylen,
			      lin);
			m0=mb.m_next;
		}
		base += (sizeof (struct mbuf *));
	}
	
	printf("\nINPUT POLICY:\n");

	base = (caddr_t)swv.ipolicyp;
	for (i=0; i<swv.skhashsiz; i++)
	{
		kvm_read(kd, base, &m0, sizeof(struct mbuf *));
		while (m0)
		{
			kvm_read(kd, (caddr_t)m0, &mb, sizeof(struct mbuf));
			si = mtod(&mb, struct sw_ipol *);
			printf("\n  %s->",
			       inet_ntoa(si->si_isrc));
			printf("%s, ",
			       inet_ntoa(si->si_idst));
			printf("policy: 0x%08x, from:\n", si->si_accpolicy);
			for (j=0; j<si->si_nosrcs; j++)
			  printf("    %s\n", inet_ntoa(si->si_osrcs[j]));
			m0=mb.m_next;
		}
		base += (sizeof (struct mbuf *));
	}

	printf("\nOUTPUT POLICY:\n");

	base = (caddr_t)swv.opolicyp;
	for (i=0; i<swv.skhashsiz; i++)
	{
		kvm_read(kd, base, &m0, sizeof(struct mbuf *));
		while (m0)
		{
			kvm_read(kd, (caddr_t)m0, &mb, sizeof(struct mbuf));
			so = mtod(&mb, struct sw_opol *);
			printf("\n  %s->",
			       inet_ntoa(so->so_isrc));
			printf("%s, ",
			       inet_ntoa(so->so_idst));
			printf("tunnel via %s->",
			       inet_ntoa(so->so_osrc));
			printf("%s\n",
			       inet_ntoa(so->so_odst));
			m0=mb.m_next;
		}
		base += (sizeof (struct mbuf *));
	}



}




#define isdot(_x)  (((_x)&0x80)|!(isprint(_x)))

/*
 * NAME
 *	xdump -- make a hex dump of a region in memory
 *
 * SYNOPSIS
 *	xdump(base, length, title)
 *	caddr_t base;
 *	int length;
 *	char *title;
 */
	
static char line[80];
static char hd[17]="0123456789ABCDEF";

#define HI(_x)  (hd[((_x)>>4)&0xF])
#define LO(_x)  (hd[(_x)&0xF])

xdump(base, length, title)
caddr_t base;
int length;
char *title;
{
	register char *bp, *hp, *cp;
	register int cnt;

	printf("%s\n", title);
	
	bp = base;

	hp = line;
	cp = line+50;
	
	for (cnt=80; cnt; line[--cnt]=' ')
	  ;

	line[49] = line[66] = '*';
	line[67] = '\0';

	while(length-- > 0)
	{
		*hp++ = HI(*bp);
		*hp++ = LO(*bp);
		hp++;
		*cp++ = isdot(*bp)?'.':(*bp);
		bp++;

		if (++cnt == 16)
		{
			cnt = 0;
			hp = line;
			cp = line+50;
			puts(line);
		}
	}

	if (cnt)
	{
		while (cnt++ < 16)
		{
			*hp++=' ';
			*hp++=' ';
			*hp++=' ';
			*cp++=' ';
		}
		puts(line);
	}

	return 0;
}

