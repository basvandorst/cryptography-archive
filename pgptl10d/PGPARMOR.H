/* pgparmor.h */

#ifdef __alpha
#define crcword unsigned int		/* if CRCBITS is 24 or 32 */
#else
#define crcword unsigned long		/* if CRCBITS is 24 or 32 */
#endif

/* Ascii armors data up to end of i or num lines, does not destroy i */
/* If lines is zero, will go until end of i */
/* Returns number of lines created */
unsigned pgp_create_armor(struct fifo *i,struct fifo *o,unsigned lines);

/* Decodes ascii armor up to end of packet (crc) */
/* Returns TRUE if CRC good, FALSE if premature end,
   bad data, or bad CRC */
int pgp_extract_armor(struct fifo *i,struct fifo *o);
