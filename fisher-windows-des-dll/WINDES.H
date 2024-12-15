/* COPYRIGHT (C) Controlled Information Environments 1988, 1989, 1990, 1991 */
/* Patent and Trademark 1975 by International Business Machines Corporation. */

#ifndef _INC_WINDES

/*ff
*******************************************************************************
*
* Name-Purpose: WinDES.h - Data Encryption Standard (DES) DLL prototypes
*
* Description:	This generic ANSI-C module employs the DES algorithm to encode
*		& decode digital data via 64-bit Electronic Code Book (ECB).
*
*		The DES algorithm uses a 64-bit user-provided private key to
*		take a 64-bit data block through 18 data manipulation stages
*		for either encryption or decryption.  The first and last stages
*		are merely simple bit transpositions - one is the inverse of
*		the other (IP & IP'). The middle 16 stages perform identical
*		complex bit manipulations that vary according to the data. Each
*		bit of the result is a function of each and any bit of both
*		the input data and the key; a change in a single key or data
*		bit has equal probability of changing any output bit.
*
*		This version designed/written by Steven Fisher CDP.
*
* External Interface:
*
*	Semaphores:	None.
*
*	Queues: 	None.
*
*	Shared Memory:	None.
*
*
* Side Effect:  None.
*
* Notes:	All comment nomenclature aheres to that in FIPS PUB 46. For
*               additional background, refer to these documents:
*		    Advanced Micro Devices Am9568 Technical Manual (1985)
*		    ANSI X3.92-1981: Data Encryption Algorithm
*		    ANSI X9.17-1985: Financial Institution Key Management
*		    FIPS PUB 46: Data Encryption Standard (15 Jan 1977)
*		    FIPS PUB 81: DES Modes of Operation
*
******************************************************************************/

/* local defines in case WINDOWS.H is unavailable */
#ifndef _INC_WINDOWS
#define FAR	        _far
typedef int             BOOL;
typedef unsigned char   BYTE;
#ifndef __BORLANDC__
typedef unsigned int    WORD;
#endif
typedef unsigned long   DWORD;
typedef char NEAR       *PSTR;
typedef char NEAR       *NPSTR;
typedef char FAR        *LPSTR;
typedef BYTE NEAR       *PBYTE;
typedef BYTE FAR        *LPBYTE;
typedef int NEAR        *PINT;
typedef int FAR         *LPINT;
typedef WORD NEAR       *PWORD;
typedef WORD FAR        *LPWORD;
typedef long NEAR       *PLONG;
typedef long FAR        *LPLONG;
typedef DWORD NEAR      *PDWORD;
typedef DWORD FAR       *LPDWORD;
typedef void FAR        *LPVOID;
#endif

/*ff
 ******************************************************************************
 *
 * Name: DesEnCrypt, DesDeCrypt
 *
 ******************************************************************************
 *
 * Description:    Performs electronic code-book (ECB) encryption via DES.
 *		   The electronic code book is a direct implementation of the
 *		   DES algorithm. The analogy to a code book arises because
 *		   the same plain text always generates the same ciphered text
 *		   for a given cryptographic key. This is a weakness in that
 *		   identical blocks of plain text generate identical blocks
 *		   of ciphered text. For randomly-accessed data, however,
 *		   this data block sequence independence is crucial.
 *
 *		   The input block is first permuted to distribute its bits
 *		   across a work buffer.  Then sixteen passes are made, where
 *                 a unique derivation of the input key is used to cipher the
 *		   the contents of the work buffer, the ciphered result is XORed
 *		   with half of the unciphered data, the left and right halves
 *		   of the work buffer swapped, and the result fed into the next
 *		   iteration. After the 16 passes are completed, the result data
 *		   is inversely permuted to undo the bit relocations caused in
 *		   the first step. Note that the positions are restored, but not
 *		   the value of the individual bits; data bit 1 is moved to bit
 *		   58, ciphered 16 times, and its new value moved back to bit 1.
 *
 *		   ECB gives an error extension characteristic which protects
 *		   against fraudulent data insertion, deletion, or alteration
 *		   in a block. A one-bit error in either the input text block
 *		   or the key causes an average error rate of 50% for its
 *                 output block. An error in one text block will not affect
 *                 any other block; there is no error extension between blocks.
 *
 *		   The input and output block size is 64 bits. Since data blocks
 *		   are independently ciphered, this mode is qualified for disk
 *		   applications employing random access.
 *
 * Synopsis:	   DesXxCrypt(key,inp,out,cnt)
 *
 *	Input:	   key - 64-bit DES key
 *                 inp - pointer to buffer of input data
 *                 out - pointer to buffer for result data
 *                 cnt - count of 64-bit blocks to be processed
 *
 *	Output:    None.
 *
 * Side Effect:	   Writes data to output buffer "out".
 *
 *****************************************************************************/
extern  VOID FAR PASCAL DesEnCrypt(LPBYTE key,LPBYTE inp,LPBYTE out,WORD cnt);
extern  VOID FAR PASCAL DesDeCrypt(LPBYTE key,LPBYTE inp,LPBYTE out,WORD cnt);

#define _INC_WINDES
#endif

/* end of header */
