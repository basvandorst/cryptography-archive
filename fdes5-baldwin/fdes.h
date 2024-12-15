/* Misc defs for the fast password transform.
 */

#define	reg	register
#define	uns	unsigned
#define unsb	uns	char
#define	unsl	uns	long


/* Types for the different ways to represent DES bit patterns.
 * Bits are always right justified within fields.
 * Bits which have lower indices in the NBS spec are stored in the
 * vax bits with less significance (e.g., Bit 1 of NBS spec is stored
 * in the bit with weight 2 ** 0 to the vax.
 */

#define	obpb1	unsb	/* One bit per byte. */
#define sbpb6	unsb	/* Six bits per byte, 6 held. */
#define sbpb6R	unsb	/* Six bits per byte Reversed order, 6 held. */
#define	sbpb24	unsl	/* Six bits per byte, 24 held. */
#define	ebpb24	unsl	/* Eight bits per bit, 24 held. */
#define	fbpb4	unsb	/* Four bits per byte, 4 held. */
#define	fbpb4R	unsb	/* Four bits per byte Reversed order, 4 held. */
