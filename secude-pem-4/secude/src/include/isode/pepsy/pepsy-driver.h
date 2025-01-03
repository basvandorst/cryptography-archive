/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/pepsy/RCS/pepsy-driver.h,v 10.0 1993/02/18 20:12:17 awy Rel $
 *
 *
 * $Log: pepsy-driver.h,v $
 * Revision 10.0  1993/02/18  20:12:17  awy
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

/*
 * common definitions for the pepsy driver routines
 */
#define PEPSY_VERSION	2

/* find the default entry for this entry - for decoding
 * - assumes its the next or the one after or the one after that.
 * old version
#define FDFLT_B(p)	((((p) + 1)->pe_type == DFLT_B) ? ((p) + 1) : \
			((((p) + 2)->pe_type == DFLT_B) ? ((p) + 2): ((p) + 3)))
 */
#define FDFLT_B(p)	fdflt_b(p)	/* try a function */

/* find the entry for this default entry - for encoding
 * call a function - to keep looking till it finds it, this is the
 * alternative to the above scheme.
 */
#define FDFLT_F(p)	fdflt_f(p)	/* have to use a function */

/* assumes encoding tables */
#define OPT_PRESENT(p, parm)	(BITTEST(parm + p->pe_ucode, p->pe_tag))

/* assumes decoding tables */
#define SET_OPT_PRESENT(p, parm)	(BITSET(*parm + p->pe_ucode, p->pe_tag))
#define CLR_OPT_PRESENT(p, parm)	(BITCLR(*parm + p->pe_ucode, p->pe_tag))

#define NO_DATA_USED	(OK + 1)
