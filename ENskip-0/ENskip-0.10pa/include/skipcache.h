/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */

typedef struct skipcache_stat
{
  u_long entries;		/* number of entries in cache */
  u_long inserted;		/* number of newly inserted entries */
  u_long updated;		/* number of updated entries */
  u_long timedout;		/* number of timedout entries */
  u_long removed;		/* number of removed entries */
  u_long lookuphits;		/* number of lookup hits */
  u_long lookuprequests;	/* number of requests sent */
} skipcache_stat_t;

extern skipcache_stat_t skipcache_stat;

typedef u_char skip_said_t[SKIP_SAIDSIZE];
typedef u_char skip_seq_t[SKIP_SEQSIZE];

struct sequence_history
{
  skip_seq_t last;		/* Last received sequence number */
  int index;			/* Index of last in history buffer */
  int len;			/* Length of history buffer */
};

struct skipcache
{
  u_char IPaddr[IPADDRSIZE];	 	/* IPv4 or IPv6 address; hash key */
  struct skipcache *next, *prev;	/* doubly linked hash chain */
  int ttl;				/* time to live in cache */
  int maxttl;				/* maximum ttl of this entry */
  int ref;				/* reference count */
  int flags;				/* flags; see below */

  /*
   * fields used by skipd
   * these fields need not be passed to the kernel
   */
  int fd;                               /* file descriptor for skipd */
  int base;                             /* base for DH */
  u_long modulus;                       /* modulus for DH */
  int moduluslen;
  u_long secretkey;                     /* secret DH key */
  int secretkeylen;
  u_long publickey;                     /* public DH key */
  int publickeylen;
  u_long lookuphosts;                   /* offset to list of lookup hosts */
  int lookuphostslen;                   /* number of lookup hosts */


  u_long Kij;				/* offset to key Kij */

  u_int filter_before[2];
  u_int filter_after[2];

  /*
   * cache information for IP packets
   * that will be converted to SKIP packets
   */
  u_int enskip_mode[2];         /* Enskip mode: See flags below */
  skip_said_t enskip_SAID;	/* SAID of IPSP header */
  int enskip_Kij_alg;		/* algorithm to use for shared key Kij */
  int enskip_Kp_alg;		/* packet encryption algorithm */
  int enskip_ICV_alg;		/* Integrity check value algorithm */
  int enskip_Comp_alg;		/* compression algorithm */
  int enskip_Kp_bytes;		/* Bytes sent with current packet key */
  int enskip_Kp_maxbytes;	/* Maximum bytes until packet key change */
  int enskip_Kp_ttl;		/* Time to live for current packet key */
  int enskip_Kp_maxttl;		/* Maximum time until packet key change */
  u_long enskip_Kp;		/* offset to packet key Kp */
  u_long enskip_Kp_Kij;		/* offset to key Kp encrypted in Kij */
  u_long enskip_MI;		/* offset to message indicator */
  skip_seq_t enskip_seqno;	/* current 64 bit sequence number */

  /*
   * cache information for SKIP packets
   * that will be converted back to normal IP packets
   */
  u_int deskip_mode[2];         /* Deskip mode: See flags below */
  skip_said_t deskip_policy[2]; /* Policy for SAID in ipsp header */
  int deskip_Kij_alg;		/* last received Kij algorithm */
  int deskip_Kp_alg;		/* last received Kp algorithm */
  u_long deskip_Kp;		/* offset to last received key Kp */
  u_long deskip_Kp_Kij;		/* offset to last received encrypted key Kp */
  struct sequence_history deskip_seqhistory;	/* received sequence numbers */
  u_long deskip_seqhistorybuf;	/* Offset to history buffer */

  /* private data for crypt algorithms */
  /*
   * Note: we do not need a second encrypt state since we cache Kp
   *       encrypted in Kij.
   */
  u_long encryptstate;		/* offset to private encrypt state */
  u_long Kp_decryptstate;	/* offset to private Kp decrypt state */
  u_long payload_decryptstate;	/* offset to private payload decrypt state */

  /*
   * this structure and its coresponding data section are
   * allocated as one area with the size (datalen + sizeof(struct skipcache))
   */
  u_char *data;			/* pointer to data section */
  int datalen;			/* length of data section */

#ifdef KERNEL
#define CACHE_QSIZE	((2 * sizeof(QUEUE)) + sizeof(void *))

  /* queue stuff */
  QUEUE inq;			/* input queue */
  QUEUE outq;			/* output queue */
  void *interface;		/* feedback interface */
#endif /* KERNEL */
};

#define SKIP_VALIDKEY	0x1	/* key stored in cache is valid */
#define SKIP_NOKEY	0x2	/* no key available */
#define SKIP_ENCAPS	0x4	/* encapsulated mode */
#define SKIP_REMOVE	0x8	/* key will be removed */
#define SKIP_REQSENT    0x10    /* key request sent to daemon */
#define SKIP_DONTAGE    0x20    /* do not expire entry */

/* Directions for skip_process */
#define SKIP_OUTPUT 0
#define SKIP_INPUT  1

#define FILTER_NONE     0
#define FILTER_IP       1
#define FILTER_IPSP     2

#define FILTER_CHECK(filter,proto) \
  (filter && \
  (((filter == FILTER_IP) && (proto != IPPROTO_IPSP)) || \
   ((filter == FILTER_IPSP) && (proto == IPPROTO_IPSP))))

/* Flags for struct skipcache enskip_mod */
#define ENSKIPMODE_NONE   0x0
#define ENSKIPMODE_TCP    0x1
#define ENSKIPMODE_UDP    0x2
#define ENSKIPMODE_ICMP   0x4
#define ENSKIPMODE_OTHER  ~(ENSKIPMODE_TCP | ENSKIPMODE_UDP | ENSKIPMODE_ICMP)
#define ENSKIPMODE_ALL    ~0

/* Flags for struct skipcache deskip_mod */
#define DESKIPMODE_NONE   0x0
#define DESKIPMODE_DESKIP 0x1

#define SKIPCACHE_MAXENTRIES      64      /* max entries in hash table */
#define SKIPCACHE_MAXTTL          1024    /* maximum time to live in cache (~1.5 h) */

#define QUEUETTL 60  /* max time to live for packets while waiting for key */
#define QUEUESIZE 50 /* max packets in queue while waiting for key */

/* constructor/ destructor */
extern int skipcache_init(int maxttl, int maxent);
extern int skipcache_exit(void);

/*
 * lookup a certain entry
 * you should release the entry afterwards to allow updates
 */
extern struct skipcache *skipcache_lookup(u_char *IPaddr);
extern int skipcache_release(struct skipcache *c);
extern int skipcache_getrequests(u_char *buf, int len);
extern int skipcache_randomKp(struct skipcache *c);

/*
 * cache manipulation functions
 */
extern int skipcache_update(struct skipcache *new);
extern int skipcache_remove(u_char *IPaddr);
extern int skipcache_flush(void);
