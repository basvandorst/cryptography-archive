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

#define FAKE_TIMES
/*
 * fake_times
 *
 *	Fakes the `times' call. Should only be used in those cases when
 *	the original times don't return any useful value.
 */


#ifdef HAS_GETTIMEOFDAY
# include	<sys/time.h>
#endif  /* HAS_GETTIMEOFDAY */


typedef unsigned long	clock_t;


static clock_t	times(
#ifdef __STDC__
	struct tms	*buffer)
#else
	buffer)
struct tms	*buffer;
#endif
{
#ifdef HAS_GETTIMEOFDAY
	struct timeval	t;
#endif /* HAS_GETTIMEOFDAY */
	clock_t		tick;


/*
	VOID times(buffer);
*/
#ifdef HAS_GETTIMEOFDAY
	VOID gettimeofday(&t, (struct timezone *) 0);
	tick = (clock_t) t.tv_usec;	/* Use miliseconds. */
#else  /* !HAS_GETTIMEOFDAY */
	tick = (clock_t) time(NULL);	/* Use the time in seconds. */
#endif /* !HAS_GETTIMEOFDAY */
	return tick;
}
