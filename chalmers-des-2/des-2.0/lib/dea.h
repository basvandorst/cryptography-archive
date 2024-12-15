/*
 *                                        _
 * Copyright (c) 1990,1991,1992,1993 Stig Ostholm.
 * All Rights Reserved
 *
 *
 * The author takes no responsibility of actions caused by the use of this
 * software and does not guarantee the correctness of the functions.
 *
 * This software may be freely distributed and modified for non-commercial use
 * as long as the copyright notice is kept. If you modify any of the files,
 * pleas add a comment indicating what is modified and who made the
 * modification.
 *
 * If you intend to use this software for commercial purposes, contact the
 * author.
 *
 * If you find any bugs or porting problems, please inform the author about
 * the problem and fixes (if there are any).
 *
 *
 * Additional restrictions due to national laws governing the use, import or
 * export of cryptographic software is the responsibility of the software user,
 * importer or exporter to follow.
 *
 *
 *                                              _
 *                                         Stig Ostholm
 *                                         Chalmers University of Technology
 *                                         Department of Computer Engineering
 *                                         S-412 96 Gothenburg
 *                                         Sweden
 *                                         ----------------------------------
 *                                         Email: ostholm@ce.chalmers.se
 *                                         Phone: +46 31 772 1703
 *                                         Fax:   +46 31 772 3663
 */

#include	"local_def.h"
#include	"fips_def.h"
#include	"tab_IP.h"
#include	"tab_IPinv.h"
#include	"tab_E.h"
#include	"tab_S_and_P.h"
#ifndef DEA_TEST
#include	"dea_conf.h"
#endif

/*
 * The funktion F
 */

#ifdef E_DATA
# ifdef S_AND_P_DATA
#  define F_DATA \
	des_cblock	b; \
	E_DATA; \
	S_AND_P_DATA
# else  /* S_AND_P_DATA */
#  define F_DATA \
	des_cblock	b; \
	E_DATA
# endif /* S_AND_P_DATA */
#else  /* E_DATA */
# ifdef S_AND_P_DATA
#  define F_DATA \
	des_cblock	b; \
	S_AND_P_DATA
# else  /* S_AND_P_DATA */
#  define F_DATA \
	des_cblock	b
# endif /* S_AND_P_DATA */
#endif /* E_DATA */

#define	F(R, KSn)  \
\
	E(b, R); \
	XOR_8(b, KSn); \
	R = S_AND_P(b)

/*
 * One iteration
 */

#ifdef F_DATA
# define ITERATION_DATA \
	F_DATA; \
	register unsigned long	r0
#else  /*  F_DATA */
# define ITERATION_DATA \
	register unsigned long	r0
#endif /*  F_DATA */

#define ITERATION(L, R, KSn) \
	r0 = R; \
	F(R, KSn); \
	R ^= L; \
	L = r0

/*
 * One encryption/decryption without initial permuation.
 */

#ifdef USE_LOOP
# ifdef USE_INC

#define CRYPT_DATA \
	ITERATION_DATA; \
	register struct des_ks_struct	*s; \
	register int			i

#define ENCRYPT(l,r,sched) \
	for (i = DES_ITERATIONS, s = &(sched)[0]; i > 0; i--, s++) { \
		ITERATION(l, r, (*s)._); \
	}

#define DECRYPT(l,r,sched) \
	for (i = DES_ITERATIONS, s = &(sched)[15]; i > 0; i--, s--) { \
		ITERATION(l, r, (*s)._); \
	}

# else  /* USE_INC */

#define CRYPT_DATA \
	ITERATION_DATA; \
	register int	i

#define ENCRYPT(l,r,sched) \
	for (i = 0; i < DES_ITERATIONS; i++) { \
		ITERATION(l, r, (sched)[i]._); \
	}

#define DECRYPT(l,r,sched) \
	for (i = DES_ITERATIONS - 1; i >= 0; i++) { \
		ITERATION(l, r, (sched)[i]._); \
	}

# endif /* USE_INC */
#else  /* USE_LOOP */
# ifdef USE_INC

#define CRYPT_DATA \
	ITERATION_DATA; \
	register struct des_ks_struct	*s

#define ENCRYPT(l,r,sched) \
	s = &(sched)[0]; \
	ITERATION(l, r, (*s++)._); \
	ITERATION(l, r, (*s++)._); \
	ITERATION(l, r, (*s++)._); \
	ITERATION(l, r, (*s++)._); \
	ITERATION(l, r, (*s++)._); \
	ITERATION(l, r, (*s++)._); \
	ITERATION(l, r, (*s++)._); \
	ITERATION(l, r, (*s++)._); \
	ITERATION(l, r, (*s++)._); \
	ITERATION(l, r, (*s++)._); \
	ITERATION(l, r, (*s++)._); \
	ITERATION(l, r, (*s++)._); \
	ITERATION(l, r, (*s++)._); \
	ITERATION(l, r, (*s++)._); \
	ITERATION(l, r, (*s++)._); \
	ITERATION(l, r, (*s)._)

#define DECRYPT(l,r,sched) \
	s = &(sched)[15]; \
	ITERATION(l, r, (*s--)._); \
	ITERATION(l, r, (*s--)._); \
	ITERATION(l, r, (*s--)._); \
	ITERATION(l, r, (*s--)._); \
	ITERATION(l, r, (*s--)._); \
	ITERATION(l, r, (*s--)._); \
	ITERATION(l, r, (*s--)._); \
	ITERATION(l, r, (*s--)._); \
	ITERATION(l, r, (*s--)._); \
	ITERATION(l, r, (*s--)._); \
	ITERATION(l, r, (*s--)._); \
	ITERATION(l, r, (*s--)._); \
	ITERATION(l, r, (*s--)._); \
	ITERATION(l, r, (*s--)._); \
	ITERATION(l, r, (*s--)._); \
	ITERATION(l, r, (*s)._)

# else  /* USE_INC */

#define CRYPT_DATA \
	ITERATION_DATA

#define ENCRYPT(l,r,sched) { \
	ITERATION(l, r, (sched)[0]._); \
	ITERATION(l, r, (sched)[1]._); \
	ITERATION(l, r, (sched)[2]._); \
	ITERATION(l, r, (sched)[3]._); \
	ITERATION(l, r, (sched)[4]._); \
	ITERATION(l, r, (sched)[5]._); \
	ITERATION(l, r, (sched)[6]._); \
	ITERATION(l, r, (sched)[7]._); \
	ITERATION(l, r, (sched)[8]._); \
	ITERATION(l, r, (sched)[9]._); \
	ITERATION(l, r, (sched)[10]._); \
	ITERATION(l, r, (sched)[11]._); \
	ITERATION(l, r, (sched)[12]._); \
	ITERATION(l, r, (sched)[13]._); \
	ITERATION(l, r, (sched)[14]._); \
	ITERATION(l, r, (sched)[15]._); \
}

#define DECRYPT(l,r,sched) { \
	ITERATION(l, r, (sched)[15]._); \
	ITERATION(l, r, (sched)[14]._); \
	ITERATION(l, r, (sched)[13]._); \
	ITERATION(l, r, (sched)[12]._); \
	ITERATION(l, r, (sched)[11]._); \
	ITERATION(l, r, (sched)[10]._); \
	ITERATION(l, r, (sched)[9]._); \
	ITERATION(l, r, (sched)[8]._); \
	ITERATION(l, r, (sched)[7]._); \
	ITERATION(l, r, (sched)[6]._); \
	ITERATION(l, r, (sched)[5]._); \
	ITERATION(l, r, (sched)[4]._); \
	ITERATION(l, r, (sched)[3]._); \
	ITERATION(l, r, (sched)[2]._); \
	ITERATION(l, r, (sched)[1]._); \
	ITERATION(l, r, (sched)[0]._); \
}

# endif /* USE_INC */
#endif /* USE_LOOP */
