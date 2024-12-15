/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
typedef struct ipsp_stat
{
  u_long in;		/* total number of IPSP packets received */
  u_long out;		/* total number of transmitted IPSP packets */
  u_long crypt_in;	/* number of encrypted packets received */
  u_long crypt_out;	/* number of encrypted packets sent */
  u_long auth_in;	/* number of authenticated packets received */
  u_long auth_out;	/* number of authenticated packets sent */
  u_long seq_in;	/* number of sequenced packets received */
  u_long seq_out;	/* number of sequenced packets sent */
  u_long comp_in;	/* number of compressed packets received */
  u_long comp_out;	/* number of compressed packets sent */
  u_long encaps_in;	/* number of received encapsulated packets */
  u_long encaps_out;	/* number of sent encapsulated packets */
  u_long enqueued_in;	/* number of enqueued incoming packets */
  u_long enqueued_out;	/* number of enqueued outgoing packets */
  u_long badipsum;	/* packets dropped due to bad ip checksum */
  u_long badpktlen;	/* packets too short */
  u_long badpolicy;     /* packets dropped due to policy violation */
  u_long badKijalg;	/* packets dropped due to unknown Kij algorithm */
  u_long badKpalg;	/* packets dropped due to unknown Kp algorithm */
  u_long badblklen;	/* payload length not multiple of block length */
  u_long badICValg;	/* packets dropped due to unknown ICV algorithm */
  u_long badICV;	/* packets dropped due to incorrect ICV */
  u_long badseq;	/* packets dropped due to incorrect sequence number */
  u_long fragdrop;      /* IP fragments dropped */
  u_long queuelimit;	/* packets dropped due to queue limits */
} ipsp_stat_t;

#ifdef KERNEL
extern struct ipsp_stat ipsp_stat;

extern int ipsp_ip2ipsp(struct skipcache *c, struct memblk *m);
extern int ipsp_ipsp2ip(struct skipcache *c, struct memblk *m, int dir);

extern int ipsp_maxheadergrowth(void);
extern int ipsp_maxtrailergrowth(void);

extern int ipsp_init(void);
extern int ipsp_exit(void);
#endif /* KERNEL */
