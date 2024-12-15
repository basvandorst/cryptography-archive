/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */

/*
 * Magic number defining supported algorithms
 */
#define SIGNALG_UNKNOWN	0
#define SIGNALG_MD5	1 	/* md5 message digest */
#define SIGNALG_MAX	2	/* Used to query maximum lengths */

/*
 * Public interface to algorithms
 */
extern int sign_init(void);		/* Initialize all crypt algorithms */
extern int sign_exit(void);		/* Cleanup all crypt algorithms */

extern int sign_icvlen(int alg);	/* Get algorithm private data len */
extern int sign_sign(int alg, u_char *Kp, int Kplen, struct memblk *m,
                     u_char *icv);

/*
 * Internal definition of an algorithm
 */
struct sign_algorithm
{
  int (*init)(void);        /* Initialization */
  int (*exit)(void);        /* Cleanup */
  /* sign sumbitted data */
  int (*sign)(u_char *Kp, int Kplen, struct memblk *m, u_char *icv);
  /* length of calculated icv */
  int (*icvlen)(void);
};

