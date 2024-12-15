/* 
 * $Header: /home/isode-consortium/isode/src/config/samples/RCS/sun4.1.2.h,v 10.0 1993/02/18 20:26:44 awy Rel $
 *
 *
 * $Log: sun4.1.2.h,v $
 * Revision 10.0  1993/02/18  20:26:44  awy
 * Release IC-R1
 *
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


#ifndef	_CONFIG_
#define	_CONFIG_

#define	BSD42			/* Berkeley UNIX */
#define	SUNOS4			/*   with Sun's enhancements */
#define	WRITEV			/*   (sort of) real Berkeley UNIX */
#define	BSD43			/*   4.3BSD or later */
#define SUNOS41			/*   4.1sun stuff */

#define	VSPRINTF		/* has vprintf(3s) routines */

#define	TCP			/* has TCP/IP */
#define	SOCKETS		/*   provided by sockets */
			/* OR */
#  undef	TLI_TCP		/*   provided by TLI */
#  undef SELECT			/*   use select(2) even without SOCKETS */

#undef USE_PP			/* Have PP so use it in QUIPU */

#undef OSISEC			/* Have OSISEC security package */

/*#define X25*/			/* X.25/CONS support for TP0 */
/*#define SUN_X25*/		/*   provided by SunNet 7.0 */

#undef TP4			/* has TP4 (or other kernal COTS) */
#undef SUN_TP4		/*   provided by SunLink OSI */
#undef SUNLINK_5_2		/*     define if using SunLink OSI release 5.2
				       or greater */
#undef SUNLINK_6_0		/*     define if using SunLink OSI release 6.0
				       or greater */
#undef SUNLINK_7_0		/*     define if using SunLink OSI release 7.0
				       or greater */

#define NOGOSIP			/* tsapd(8) will only handle calls
				   discriminated at Transport (TSEL);
				   undef'ing this is deprecated */
/*
#ifndef DEBUG
#define DEBUG
#endif
*/

#define	GETDENTS		/* has getdirent(2) call */
#define	SCANDIR			/* has scandir(3) & alphasort(3) */
#define	NFS			/* network filesystem -- has getdirentries */
#define TEMPNAM                 /* has tempnam (3) call */

#endif /* _CONFIG_ */
