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
 * ip_swipe.h include file
 */

/*
 * Definitions for swIPe header
 */

#define IPIP_PLAIN	0		/* simple encapsulation */
#define IPIP_AUTH	1		/* authenticated */
#define IPIP_CRYPT	2		/* encrypted (rare) */
#define IPIP_AE		3		/* authenticated and encrypted */

#define IPIP_GETSNDKEY	254		/* used in pseudo-packet passed */
#define IPIP_GETRCVKEY  253		/* up by the kernel */

#define IPIP_MM1	249		/* key management */
#define IPIP_MM2	248		/* key management */
#define IPIP_MM3	247		/* key management */
#define IPIP_MM4	246		/* key management */
#define IPIP_MM5	245		/* key management */

#define IPIP_NOHDRMASK  0xF0
#define IPIP_NOHDRVAL   (IPVERSION << 4)

#define SWIPE_A_MD5_128	1		/* MD5 authentication, 128bit sign */
#define SWIPE_A_MD5_64	2		/* MD5 authentication, 64bit sign */
#define SWIPE_A_MD5_32	3		/* MD5 authentication, 32bit sign */
#define SWIPE_A_SHA_160 4		/* SHA authentication, 160bit sign */
#define SWIPE_A_SHA_32  5		/* SHA authentication, 32bit sign */

#define SWIPE_E_DES	1		/* DES encryption */
#define SWIPE_E_3DES	2		/* Triple-DES (EDE-mode) encryption */
#define SWIPE_E_IDEA	3		/* IDEA encryption */
#define SWIPE_E_SKJ	4		/* Skipjack encryption (requires
					   specialized hardware) */

#define SWIPE_S_STATIC	1		/* Sneakernet key exchange */
#define SWIPE_S_DH	2		/* Diffie-Hellman key exchange */
#define SWIPE_S_FF	3		/* Firefly key management */

/*
 * ioctl()s for the device driver
 */

#ifdef __STDC__
#define SWIPSSNDKEYS		_IOWR('w', 0, struct swkey_ent)
#define SWIPGSNDKEYS		_IOWR('w', 1, struct swkey_ent)
#define SWIPSRCVKEYS		_IOWR('w', 2, struct swkey_ent)
#define SWIPGRCVKEYS		_IOWR('w', 3, struct swkey_ent)
#define SWIPSOPOL		_IOWR('w', 4, struct sw_opol)
#define SWIPGOPOL		_IOWR('w', 5, struct sw_opol)
#define SWIPSIPOL		_IOWR('w', 6, struct sw_ipol)
#define SWIPGIPOL		_IOWR('w', 7, struct sw_ipol)
#define SWIPGSWVARS		_IOWR('w', 8, struct swipevars)
#else
#define SWIPSSNDKEYS		_IOWR(w, 0, struct swkey_ent)
#define SWIPGSNDKEYS		_IOWR(w, 1, struct swkey_ent)
#define SWIPSRCVKEYS		_IOWR(w, 2, struct swkey_ent)
#define SWIPGRCVKEYS		_IOWR(w, 3, struct swkey_ent)
#define SWIPSOPOL		_IOWR(w, 4, struct sw_opol)
#define SWIPGOPOL		_IOWR(w, 5, struct sw_opol)
#define SWIPSIPOL		_IOWR(w, 6, struct sw_ipol)
#define SWIPGIPOL		_IOWR(w, 7, struct sw_ipol)
#define SWIPGSWVARS		_IOWR(w, 8, struct swipevars)
#endif

#define MAXAUTHSIGNATURE	20	/* max signature size, octets */
#define MAXKEYSIZE		32	/* max key size, octets */

struct swipe
{
	u_char	sw_type;		/* packet type */
	u_char	sw_hlen;		/* header length, in doublewords */
	u_short	sw_polid;		/* policy identifier */
	u_long	sw_seqnum;		/* sequence number */
	u_char	sw_auth[MAXAUTHSIGNATURE]; /* Authentication data */
};

struct swipe_key
{
	u_short sk_keylen;	/* this is value-result!!! */
	u_short	sk_algo;
	union
	{
		u_char	Sk_data[MAXKEYSIZE];
		struct
		{
			u_char	Sk_deskey[8];
			caddr_t	Sk_keysched;
		}S;
	}SWU;
};

#define	sk_data		SWU.Sk_data
#define sk_deskey	SWU.S.Sk_deskey
#define sk_keysched	SWU.S.Sk_keysched

/*
 * |swIPe| key entry for a pair of hosts. To save time in lookups,
 * we store both keys here. Since this takes up an mbuf (112 bytes),
 * and there are 6 doublewords (24 bytes) before the key data start,
 * there are 88 free bytes for the two keys, or 44 bytes per key.
 * Each key header takes up four bytes, so each key proper may be up
 * to 40 bytes, or 320 bits. This is enough for most applications.
 * If we move to longer addresses, or longer keys, and they don't
 * fit inside an ordinary mbuf, get a cluster!!!
 *
 * se_authkey and se_cryptkey must be set to point into the mbuf
 * (starting at se_data[0]). If either pointer is null, there is no 
 * associated key.
 *
 * Since we now want this to work with NET2 (BSDI), we have to 
 * make sure everything fits inside a 108-byte mbuf. 
 * Keys are now limited to 32 octets.
 *
 * Contrary to an earlier implementation, this table is consulted 
 * only after we've decided who the recipient of the outer (encapsulating)
 * packet is going to be, and hence the src/dst pair refers to the
 * "osrc/odst" of JI's notebook, p. 150, that is, the endpoints of the tunnel.
 */


struct swkey_ent
{
	u_long se_hash;			/* hash, for fast lookups */
	struct in_addr se_osrc;		/* tunnel source address */
	struct in_addr se_odst;		/* tunnel destination address */
	u_long se_seqnum;		/* next sequence number */
	u_long se_timeout;		/* key aging */
	struct swipe_key se_authkey;	/* key to use for authentication */
	struct swipe_key se_cryptkey;	/* key to use for encryption */
	/*
	 * there are MMAXOFF-MMINOFF-92 (16 or 20) bytes to spare
	 * when this is stored in an mbuf
	 */
};


#define SWP_REQUIRECRYPT	0x0001
#define SWP_REQUIREAUTH		0x0002

#define MAXOSRCS	20
struct sw_ipol
{
	u_long si_hash;			/* hash, for fast lookups */
	struct in_addr si_isrc;		/* `inside' packet src... */
	struct in_addr si_idst;		/* ... and destination (us) */
	u_long si_accpolicy;		/* bitfield */
	u_long si_nosrcs;		/* number of osrcs, 0 == reject all */
	struct in_addr si_osrcs[MAXOSRCS];
};

struct sw_opol
{
	u_long so_hash;			/* hash, for fast lookups */
	struct in_addr so_isrc;		/* `inside' packet src... */
	struct in_addr so_idst;		/* ... and destination  */
	struct in_addr so_osrc;		/* tunnel it using this interface */
	struct in_addr so_odst;		/* tunnel it via this host */
};

/*
 * _s and _d are of type struct in_addr. 
 * Byte sex is not really important, since we are only using it to hash,
 * and compare it against values from the same host.
 */

/*
 * Selection flags -- what to match against when searching the hash tables.
 */

#define SWSEL_SRC	0x01
#define SWSEL_DST	0x02

/* 
 * only search according to the source OR destination address.
 *
 * IT IS AN ERROR TO HAVE BOTH FLAGS SET!!!
 */

#define SWIPEHASH(_s, _d, _sel) \
 ((_sel)==SWSEL_SRC?ntohl((_s).s_addr):ntohl((_d).s_addr))

#define SWIPEHASH2(_s, _d) (ntohl((_s).s_addr)+ntohl((_d).s_addr))
/*
 * if we were ever searcing on both...
 *
#define SWIPEHASH(_s, _d, _sel) \
( (((_sel)&SWSEL_SRC)?ntohl((_s).s_addr):0) + \
  (((_sel)&SWSEL_DST)?ntohl((_d).s_addr):0) )
 */
 
/*
 * This really belongs in <sys/mbuf.h>
 */

#define	MT_SWKEYENT	18	/* swipe key entry  */
#define MT_SWKEYSCHEDS	19	/* memory allocated to key schedules */
#define MT_SWIPOLICY	20	/* swIPe input policy engine buffers */
#define MT_SWOPOLICY	21	/* swIPe output policy engine buffers */

/*
 * This really belongs in <netinet/ip_var.h>
 */

#define IP_SWIPED	0x8

/*
 * This really belongs in <netinet/ip.h>
 */

#define IPPROTO_IPIP	94

struct swipestat
{
	u_long ipip_badheader;
	u_long ipip_badrcvcrypt;
	u_long ipip_badrcvauth;
};

struct swipevars
{
	int skhashsiz;
	struct mbuf **keysndp;
	struct mbuf **keyrcvp;
	struct mbuf **ipolicyp;
	struct mbuf **opolicyp;
#ifdef DEBUGSW
	unsigned long *debugsw;
#endif
#ifdef SWIPEDEBUG
	unsigned long *swipedebug;
#endif
};


#ifdef KERNEL
#define SKHASHSIZ	256
#if	(SKHASHSIZ & (SKHASHSIZ - 1)) == 0
#define SKHASHMOD(h)	((h) & (SKHASHSIZ - 1))
#else
#define SKHASHMOD(h)	((h) % SKHASHSIZ)
#endif

struct mbuf *keysnd[SKHASHSIZ];
struct mbuf *keyrcv[SKHASHSIZ];

struct mbuf *ipolicy[SKHASHSIZ];
struct mbuf *opolicy[SKHASHSIZ];

struct swipestat swipestat;

extern struct swipevars swipevars;

#endif


#ifdef __STDC__
int ipip_swipecheck(struct mbuf *m, struct ifnet *ifp);
int swioctl(int cmd, caddr_t data);
struct swkey_ent *swipe_get(struct in_addr src, struct in_addr dst, struct mbuf *where[], int flags);
struct swkey_ent *swipe_getoralloc(struct in_addr src, struct in_addr dst, struct mbuf *where[], int flags);
struct sw_opol *swipe_getopol(struct in_addr src, struct in_addr dst);
struct sw_opol *swipe_getorallocopol(struct in_addr src, struct in_addr dst);
struct sw_ipol *swipe_getipol(struct in_addr src, struct in_addr dst);
struct sw_ipol *swipe_getorallocipol(struct in_addr src, struct in_addr dst);
int swipe_output(struct mbuf *m, struct swkey_ent *se, struct route *ro);
void md5mbuf(struct mbuf *m0, struct swipe_key *key, caddr_t digest);
#else
int ipip_swipecheck();
int swioctl();
struct swkey_ent *swipe_get();
struct swkey_ent *swipe_getoralloc();
struct sw_opol *swipe_getopol();
struct sw_opol *swipe_getorallocopol();
struct sw_ipol *swipe_getipol();
struct sw_ipol *swipe_getorallocipol();
int swipe_output();
void md5mbuf();
#endif


