/*  CRC-32 version 1.04 by Craig Bruce, 05-Dec-1994
**
**  Based on "File Verification Using CRC" by Mark R. Nelson in Dr. Dobb's
**  Journal, May 1992, pp. 64-67.  This program DOES generate the same CRC
**  values as ZMODEM and PKZIP
**
**  v1.00: original release.
**  v1.01: fixed printf formats.
**  v1.02: fixed something else.
**  v1.03: replaced CRC constant table by generator function.
**  v1.04: reformatted code, made ANSI C.  05-Dec-1994.
**
**    modified by David Vincenzetti <vince@dsi.unimi.it> for STEL 951228
*/

static unsigned long crcTable[256];

#ifdef TEST
unsigned long	crc32val;
#else
extern unsigned long	crc32val;
#endif

void
UpdateCRC(buf, len)
register char	*buf;
register int	len;
{
	register	i;
	int		c;

	for (i = 0; i < len; i++)  {
		c = buf[i];
		crc32val = ((crc32val >> 8) & 0x00FFFFFF) ^
			crcTable[ (crc32val ^ c) & 0xFF ];
	}
}

void
crcgen()
{
    unsigned long	crc, poly;
    register		i, j;

	poly = 0xEDB88320L;

	for (i = 0; i < 256; i++) {
		crc = i;

		for (j = 8; j > 0; j--) {
			if (crc & 1)
				crc = (crc >> 1) ^ poly;
			else
				crc >>= 1;
		}
		crcTable[i] = crc;
	}

	crc32val = 0L;
}

#ifdef TEST
main()
{
	char	buf[128];
	int	len;

	crcgen();
	while ((len = read(0, buf, sizeof(buf))) > 0)
		UpdateCRC(buf, len);
	
	if (len == -1) {
		perror("read()");
		exit(1);
	}

	printf("CRC32 = %x\n", crc32val);
	return(0);
}
#endif
