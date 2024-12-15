
/*
 * @(#)remailer.h	1.2 6/29/94
 *
 *	(c) Copyright 1993-1994 by Mark Grant. All right reserved.
 *	The author assumes no liability for damages resulting from the 
 *	use of this software, even if the damage results from defects in
 *	this software. No warranty is expressed or implied.
 *
 *	This software is being distributed under the GNU Public Licence,
 *	see the file COPYING for more details.
 *
 *			- Mark Grant (mark@unicorn.com) 29/6/94
 *
 */

typedef struct _remailer {

	struct	_remailer	*prev;
	struct	_remailer	*next;

	char	*name;
	char	*email;
	char	*key_id;
	word16	flags;

} REMAILER;

/* Flag definitions */

#define	RM_RRT		0x0001	/* Request-Remailing-To: */
#define	RM_ANT		0x0002	/* Anon-To: */
#define	RM_CUT		0x0004	/* Cutmarks: */
#define	RM_PGP		0x0008	/* Supports PGP */
#define	RM_ENC		0x0010	/* Requires Encrypted: PGP */
#define	RM_TO		0x0020	/* Supports To: format */

/* Remailer list structure */

typedef struct _rmlist {

	/* Linked list */

	struct _rmlist	*next;	
	struct _rmlist	*prev;

	/* Perhaps we've got a choice here */

	struct	_rmlist	*random;

	/* Aha, the remailer in question */

	REMAILER	*mailer;

} RMLIST;

/* Route structure */

typedef struct _route {

	struct	_route	*next;
	struct	_route	*prev;

	char	*name;
	RMLIST	*route;

} ROUTE;

