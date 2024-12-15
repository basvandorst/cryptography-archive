/*
 * $Author: vince $
 * $Header: /staff/vince/src/atp/RCS/sub2.c,v 1.4 1992/03/04 09:40:19 vince Exp vince $
 * $Date: 1992/03/04 09:40:19 $
 * $Revision: 1.4 $
 * $Locker: vince $
 * $Log: sub2.c,v $
 * Revision 1.4  1992/03/04  09:40:19  vince
 * AutoDeposit
 *
 * Revision 1.3  1992/02/13  11:52:43  vince
 * AutoDeposit
 *
 * Revision 1.2  1992/02/13  11:20:53  vince
 * AutoDeposit
 *
 * Revision 1.1  1992/02/11  11:47:54  vince
 * Initial revision
 *
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: sub2.c,v 1.4 1992/03/04 09:40:19 vince Exp vince $";
#endif	lint

#include "atp.h"

/* Local functions */
unsigned char	*getdig();
unsigned char	*getIV();

int		do_checksum();

/*
 * Calculate the checksum from a file handler.
 *
 * Returns: boolen.
 */
int	do_checksum(in)
int	in;
{
	int		len;
	char		buf[10240];

#ifdef MD4
	MD4Init(&mdContext);
#else /* Defaults to CRC32 */
	crc32val = 0L;
#endif
	while ((len = read(in, buf, sizeof(buf))) > 0)
#ifdef MD4
		MD4Update(&mdContext, buf, len);
#else /* Defaults to CRC32 */
		UpdateCRC(buf, len);
#endif
	if (len == -1)
		return(1);
#ifdef MD4
	MD4Final(&mdContext);
#endif
	return(0);
}

/*
 * getdig() converts a mdContext.digest field into hex ASCII readable form.
 * getIV() converts a C_Block struct into hex ASCII readable form.
 * 
 * Returns: char pointer 
 */
#define HEXPAT	\
	"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
#define HEXPAT2	"%02x%02x%02x%02x%02x%02x%02x%02x"
unsigned char	*getdig(dig)
unsigned char	*dig;
{
	static unsigned char	buf[33];

	sprintf(buf, HEXPAT,
		dig[0],  dig[1],  dig[2],  dig[3],
		dig[4],  dig[5],  dig[6],  dig[7],
		dig[8],  dig[9],  dig[10], dig[11],
		dig[12], dig[13], dig[14], dig[15]);
	buf[32] = '\0';
	return(buf);
}
unsigned char	*getIV(IV)
C_Block		*IV;
{
	static unsigned char	buf[17];

	sprintf(buf, HEXPAT,
		IV->data[0],  IV->data[1],  IV->data[2],  IV->data[3],
		IV->data[4],  IV->data[5],  IV->data[6],  IV->data[7]);
	buf[16] = '\0';
	return(buf);
}
