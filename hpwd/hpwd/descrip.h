#ifndef __DESCRIP_LOADED
#define __DESCRIP_LOADED	1

/*
 * This definition lifted from the VAXC  descrip.h  for descriptor definition.
 */


/*
 *	DESCRIP.H - V3.0-003 - Argument Descriptor Formats
 *	(Based on the VAX Procedure Calling and Condition Handling Standard,
 *	 Revision 9.4 [13 March 1984];
 *	 see the "Introduction to VMS System Routines" manual for further
 *	 information.)
 */


/*
 *	Fixed-Length Descriptor:
 */
struct	dsc$descriptor_s
{
	unsigned short	dsc$w_length;	/* length of data item in bytes */
	unsigned char	dsc$b_dtype;	/* data type code */
	unsigned char	dsc$b_class;	/* descriptor class code */
	char		*dsc$a_pointer;	/* address of first byte of data */
					/* storage */
};



/*
 *	A simple macro to construct a string descriptor:
 */
#define $DESCRIPTOR(name,string)	struct dsc$descriptor_s name = { sizeof(string)-1, DSC$K_DTYPE_T, DSC$K_CLASS_S, string }

#endif					/* __DESCRIP_LOADED */
