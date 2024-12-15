/*
 * pgpAnnotate.h -- This is the list of known annotations.  Each
 * annotation has its own number, to make it unique.  In addition,
 * this list really should not change too much over time.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU> and Colin Plumb
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpAnnotate.h,v 1.10 1997/09/16 00:47:02 hal Exp $
 */

#ifndef Included_pgpAnnotate_h
#define Included_pgpAnnotate_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS

/* Convert a scope type (via a begin) to a string */
const char  *pgpScopeName (int type);

/* NOTE: Begin scope always ends in 0x00, End scope always ends in 0x01 */
#define PGP_IS_SCOPE_MARKER(x)	(((x) & 0xfe) == 0x00)
#define PGP_IS_END_SCOPE(x)		(((x) & 0xff) == 0x01)
#define PGP_IS_BEGIN_SCOPE(x)	(((x) & 0xff) == 0x00)

/*
 * Given an annotation type, update the current scope depth accordingly.
 * Can be followed with "pgpAssert(depth != -1)".
 */
#define PGP_SCOPE_DEPTH_UPDATE(depth,x) \
	if (PGP_IS_SCOPE_MARKER(x)) \
		(depth) += PGP_IS_BEGIN_SCOPE(x) ? 1 : -1

/* Commit to sending the current packet: e.g., request a callback! */
#define PGPANN_COMMIT			0x002
/*
 * There are four possible packet-ended-too-early cases, based on
 * two binary options.  The first binary option is whether the
 * end is caused by EOF or a short packet length.  (E.g. an
 * encrypted packet that does not at least contain an IV.)
 * These are called the _TRUNCATED and _SHORT cases, respectively.
 * The second is whether the problem applies to the current scope
 * or not.  I.e. is it at the current parsing level, or one level
 * down?  These are marked as _SCOPE_ and _PACKET_, respectively.
 */
#define PGPANN_PACKET_TRUNCATED		0x010
#define PGPANN_PACKET_SHORT		0x011
#define PGPANN_SCOPE_TRUNCATED		0x012
#define PGPANN_SCOPE_SHORT		0x013

/* A signature is too damn big to pass as an annotation */
#define PGPANN_SIGNATURE_TOO_BIG	0x020
/* An encrypted session key is too damn big */
#define PGPANN_ESK_TOO_BIG		0x021

/* For a FileWrite module: start a new file. */
#define PGPANN_NEWFILE_START		0x030

/* Memory buffer output module; size of buffer, bytes requested to write */
#define PGPANN_MEM_BUFSIZE			0x40
#define PGPANN_MEM_BYTECOUNT		0x41
#define PGPANN_MEM_ENABLE			0x42
#define PGPANN_MEM_MAXSIZE			0x43
#define PGPANN_MEM_MEMBUF			0x44
#define PGPANN_MEM_PREPEND			0x45

/* Status requests of the ascii armor parser */
#define PGPANN_PARSEASC_MSG_COUNT	0x050
#define PGPANN_PARSEASC_MSG_LEFT	0x051
#define PGPANN_PARSEASC_MSG_CURRENT	0x052
#define PGPANN_PARSEASC_MSG_PARTS	0x053
#define PGPANN_PARSEASC_MSG_PARTINFO	0x054

/* Parser Callbacks */
#define PGPANN_PARSER_EATIT		0x102
#define PGPANN_PARSER_PASSTHROUGH	0x103
#define PGPANN_PARSER_PROCESS		0x104
#define PGPANN_PARSER_RECURSE		0x105

/* Unrecognized packet type (EATIT, PASSTHROUGH, PROCESS options only) */
#define PGPANN_UNKNOWN_BEGIN		0x200
#define PGPANN_UNKNOWN_END		0x201

/* Literal Packet information */
#define PGPANN_LITERAL_BEGIN		0x300
#define PGPANN_LITERAL_END		0x301
#define PGPANN_LITERAL_TYPE		0x302
#define PGPANN_LITERAL_NAME		0x303
#define PGPANN_LITERAL_TIMESTAMP	0x304

/* Cipher Packet Information */
#define PGPANN_CIPHER_BEGIN		0x400
#define PGPANN_CIPHER_END		0x401
#define PGPANN_CIPHER_NOTEXT		0x402
#define PGPANN_SKCIPHER_ESK		0x403
#define PGPANN_PKCIPHER_ESK		0x404

/* Compressed Packet Information */
#define PGPANN_COMPRESSED_BEGIN		0x500
#define PGPANN_COMPRESSED_END		0x501

/* Comment Packet Information */
#define PGPANN_COMMENT_BEGIN		0x600
#define PGPANN_COMMENT_END		0x601

/* Signed messages */
#define PGPANN_SIGNED_BEGIN		0x900
#define PGPANN_SIGNED_END		0x901
#define PGPANN_SIGNED_SIG		0x902
#define PGPANN_SIGNED_SIG2		0x903
#define PGPANN_SIGNED_SEP		0x904
	
/* Hash Packets */
#define PGPANN_HASH_REQUEST		0x910	/* Callback */
#define PGPANN_HASH_VALUE		0x920	/* Double callback (!) */

/* PGP Key Data */
#define PGPANN_PGPKEY_BEGIN		0xF00
#define PGPANN_PGPKEY_END		0xF01

/* Not a packet at all (EATIT, PASSTHROUGH options only) */
#define PGPANN_NONPACKET_BEGIN		0x1000
#define PGPANN_NONPACKET_END		0x1001

/* From the Armor Parser.  text not in armor */
#define PGPANN_NONPGP_BEGIN		0x1100
#define PGPANN_NONPGP_END		0x1101

/* Ascii Armor'ed data */
#define PGPANN_ARMOR_BEGIN		0x1200
#define PGPANN_ARMOR_END		0x1201
#define PGPANN_ARMOR_PART		0x1202
#define PGPANN_ARMOR_BADLINE		0x1203
#define PGPANN_ARMOR_BADHEADER		0x1204
#define PGPANN_ARMOR_TOOLONG		0x1205
#define PGPANN_ARMOR_NOCRC		0x1206
#define PGPANN_ARMOR_CRCCANT		0x1207
#define PGPANN_ARMOR_CRCBAD		0x1208

/* Clearsigned Data */
#define PGPANN_CLEARSIG_BEGIN		0x1300
#define PGPANN_CLEARSIG_END		0x1301

/* Main input scope -- only an application should send this annotation */
#define PGPANN_INPUT_BEGIN		0x1400
#define PGPANN_INPUT_END		0x1401

/* Independent files. */
#define PGPANN_FILE_BEGIN		0x1500
#define PGPANN_FILE_END			0x1501

/* Armor Multipart messages */
#define PGPANN_MULTIARMOR_BEGIN		0x1600
#define PGPANN_MULTIARMOR_END		0x1601

/* PGP-MIME information */
/*
 * HEADER_SIZE tells size of mime headers, offset of body, assuming newlines
 * are 1 char long.  Add HEADER_LINES if output is padded to two chars per
 * newline
 */
#define PGPANN_PGPMIME_HEADER_SIZE	0x1610
#define PGPANN_PGPMIME_HEADER_LINES	0x1611
#define PGPANN_PGPMIME_SEPARATOR	0x1612


PGP_END_C_DECLARATIONS

#endif /* Included_pgpAnnotate_h */
