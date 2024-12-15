/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	pgpImage.h - header for PGP image index definitions
	

	$Id: pgpImage.h,v 1.5 1997/10/15 22:54:09 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_pgpImage_h	/* [ */
#define Included_pgpImage_h


// bitmap images index definitions

#define IDX_NONE			-1
#define IDX_INVALID			0
#define IDX_PUSHED			1
#define IDX_VALID			2
#define IDX_VALIDPUSHED		3
#define IDX_AXIOMATIC		4
#define IDX_AXIOMATICPUSHED	5
#define IDX_RSAPUBKEY		6
#define IDX_RSAPUBDISKEY	7
#define IDX_RSAPUBREVKEY	8
#define IDX_RSAPUBEXPKEY	9
#define IDX_RSASECKEY		10
#define IDX_RSASECDISKEY	11
#define IDX_RSASECREVKEY	12
#define IDX_RSASECEXPKEY	13
#define IDX_DSAPUBKEY		14
#define IDX_DSAPUBDISKEY	15
#define IDX_DSAPUBREVKEY	16
#define IDX_DSAPUBEXPKEY	17
#define IDX_DSASECKEY		18
#define IDX_DSASECDISKEY	19
#define IDX_DSASECREVKEY	20
#define IDX_DSASECEXPKEY	21
#define IDX_RSAUSERID		22
#define IDX_DSAUSERID		23
#define IDX_INVALIDUSERID	24
#define IDX_CERT			25
#define IDX_REVCERT			26
#define IDX_BADCERT			27
#define IDX_EXPORTCERT		28
#define IDX_METACERT		29
#define IDX_EXPORTMETACERT	30
#define IDX_GROUP			31
#define IDX_NOADK			32
#define	IDX_NOADKPUSHED		33
#define IDX_ADK				34
#define IDX_ADKPUSHED		35
#define NUM_BITMAPS			36

#endif /* ] Included_pgpImage_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

