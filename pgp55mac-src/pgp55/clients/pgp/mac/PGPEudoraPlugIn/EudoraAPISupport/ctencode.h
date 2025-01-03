/* ======================================================================

   Eudora Extended Message Services API SDK 1.0b3 (July 11 1996)
   This SDK supports EMSAPI version 1
   Copyright 1995, 1996 QUALCOMM Inc.
   Send comments and questions to <emsapi-info@qualcomm.com>

   Content Transfer Encoding functions
	 Code extracted from Eudora. Based on Steve Dorner's code.
                 
   Last Edited: July 23, 1996
   
   Filename: ctencode.h

   Myra Callen <t_callen@qualcomm.com>
   
*/


#ifndef CTENCODE_H
#define CTENCODE_H

/*
 * Return values for encoders and decoders
 */
#define kCTEDoneOK 0
#define kCTEFailed 1


typedef unsigned char *UPtr, *PStr;

/* ----------------- Quoted Printable ------------------*/
/*
 * state buffer for quoted-printable decoding
 */
typedef enum
{
	qpNormal,
	qpEqual,
	qpByte1
}	QPStates;

typedef struct
{
	QPStates	state;
	Byte 			lastChar;
	long			spaceCount;
} DecQP, *DecQPPtr, **DecQPHandle;

/*
 * to do the quoted-printable encoding/decoding
 */
long EncodeQP(UPtr binPtr,long binLen,UPtr qpPtr,long *qpBufSize,long *eQP);
long DecodeQP(UPtr qpPtr,long qpLen,UPtr binPtr,long *binLen,
			DecQPPtr dQP,long *decErrCnt);

/*
 * encoder/decoder call types
 */
typedef enum
{
	kDecodeInit,
	kDecodeData,
	kDecodeDone,
	kDecodeDispose
} CallType;

#ifdef JUNK
typedef struct DecoderPB DecoderPB, *DecoderPBPtr, **DecoderPBHandle;

/*
 * for decoders and file savers
 */
struct DecoderPB
{
	UPtr input;
	long inlen;
	UPtr output;
	long outlen;
	long refCon;
	Boolean text;
};

#endif



/* ---------------- Base 64 CTE ----------------*/

#define ABS(x)					((x)<0 ? -(x) : (x))


/*
 * state buffer for encoding
 */
typedef struct
{
	Byte partial[4];
	short partialCount;
	short bytesOnLine;
} Enc64, *Enc64Ptr, **Enc64Handle;

/*
 * state buffer for decoding
 */
typedef struct
{
	short decoderState;	/* which of 4 bytes are we seeing now? */
	long invalCount;		/* how many bad chars found so far? */
	long padCount;			/* how many pad chars found so far? */
	Byte partial;	/* partially decoded byte from/for last/next time */
	Boolean wasCR;			/* was the last character a carriage return? */
} Dec64, *Dec64Ptr, **Dec64Handle;


/*
 * to do the base 64 encoding/decoding
 */
short Encode64(UPtr bin, long len,
		UPtr sixFour, long *sixFourLen, Enc64Ptr e64 );
short Decode64(UPtr sixFour,
		long sixFourLen, UPtr bin, long *binLen,
					Dec64Ptr d64, long *decErrCnt);
					
					
/* ----------------- Identify the CTE ------------------- */
#define kCTEncodeHdrPStr	"\pContent-Transfer-Encoding:"
#define kCTEncodeHdrCStr	"Content-Transfer-Encoding:"
#define kCTEncodeHdrLen   26
#define kCTEbase64TypeStr "\pbase64"
#define kCTEQPTypeStr	  	"\pquoted-printable"
#define kCTE7bitTypeStr		"\p7bit"
#define kCTE8bitTypeStr		"\p8bit"
#define kCTEbinaryTypeStr	"\pbinary"


typedef enum
{ 
  kCTE7bit,
  kCTE8bit,
  kCTEbinary,
  kCTEbase64,
  kCTEQP,
  kCTEUnknown
} CTEType;

CTEType ParseCTEHeader(Handle headerH);
Handle GenCTEHeader(CTEType encType);

#endif /* ENCODING_H */
