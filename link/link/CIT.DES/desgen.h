/*
 *	desgen.h - typdefs for the desgen DES table compiler program.
 *
 *	RCS ID: $Header: desgen.h,v 2.1 90/07/18 10:56:33 lpb Exp $
 *
 *
 *  Author: Lawrence Brown <lpb@cs.adfa.oz.au>      Dec 1987
 *      Computer Science, UC UNSW, Australian Defence Force Academy,
 *          Canberra, ACT 2600, Australia.
 *
 *  Copyright 1989 by Lawrence Brown and CITRAD. All rights reserved.
 *      This program may not be sold or used as inducement to buy a
 *      product without the written permission of the author.
 */

/*
 *	sp_desc - describes a permutation or substitution table and its
 *		associated compiled function, its parameters, their types
 *		and valid bit positions
 */

typedef	struct {
	char	*fn;		/* name of the macro the perm is compiled to */
	char	*perm;		/* pointer to the perm table to be compiled  */
	int	owords;		/* number of words needed to hold output bits*/
	char	*otype;		/* string naming the type of output words    */
	int	omsb;		/* number of MSB used in the output words    */
	int	obits;		/* number of DES bits used in output words   */
	int	iwords;		/* number of words needed to hold input bits */
	char	*itype;		/* string naming the type of input words     */
	int	imsb;		/* number of MSB used in the input words     */
	int	ibits;		/* number of DES bits used in input words    */
	unsigned short	flags;	/* assorted flags specifying options reqd    */
	} sp_desc;

/*
 *	Valid flags for sp_desc.flags
 */
#define O_ALIGN	0x01	/* may have byte alignment probs due to save as Long  */
#define O_SWAP	0x02	/* need to byte swap output on LITTLE_ENDIAN machines */
#define O_PE	0x04	/* concat sp_desc.perm with expansion function E      */
#define O_REG	0x08	/* pass out as a set of reg variables, not an array   */

#define I_ALIGN	0x100	/* may have byte alignment probs due to save as Long  */
#define I_SWAP	0x200	/* need to byte swap input on LITTLE_ENDIAN machines  */
#define I_EP	0x400	/* concat expansion function E with sp_desc.perm      */
#define I_REG	0x800	/* pass in as a set of reg variables, not an array    */

