/*
 * pgpMsg.h -- A file of PGP Messages
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpMsg.h,v 1.6 1997/06/16 22:48:46 lloyd Exp $
 */

#ifndef Included_pgpMsg_h
#define Included_pgpMsg_h

#ifndef PGPMSG

PGP_BEGIN_C_DECLARATIONS

#define PGPMSG(num, string)	/* nothing */
const char  *pgpmsgString (int type);

PGP_END_C_DECLARATIONS

#endif

#define PGPMSG_NONE	0
PGPMSG (PGPMSG_NONE,		"")

#define PGPMSG_NO_KEY	1000
PGPMSG (PGPMSG_NO_KEY,		"Error getting public key")

#define PGPMSG_SIG_UNKNOWN_KEYID	1050
PGPMSG (PGPMSG_SIG_UNKNOWN_KEYID,
	"Signature by unknown keyid: %.*s")
#define PGPMSG_SIG_ERROR	1051
PGPMSG (PGPMSG_SIG_ERROR,	"Signature check got an error")
#define PGPMSG_SIG_NOVERIFY	1052
PGPMSG (PGPMSG_SIG_NOVERIFY,	"Signature verification unsuccessful")
#define PGPMSG_SIG_ADD_ERROR	1053
PGPMSG (PGPMSG_SIG_ADD_ERROR,	"Error adding Sig")
#define PGPMSG_SIG_NO_CHECK	1054
PGPMSG (PGPMSG_SIG_NO_CHECK,
	"None of the signatures were understood; can't check")
#define PGPMSG_SIG_BADHASH	1055
PGPMSG (PGPMSG_SIG_BADHASH,
	"unknown hash: unable to verify signature")
#define PGPMSG_SIG_NOSIGS	1056
PGPMSG (PGPMSG_SIG_NOSIGS,	"No signatures to check")

#define PGPMSG_ENCRYPTED_PASSPHRASE	1100
PGPMSG (PGPMSG_ENCRYPTED_PASSPHRASE,
	"This message is encrypted with a pass phrase")

#define PGPMSG_UNBALANCED_SCOPE	1150
PGPMSG (PGPMSG_UNBALANCED_SCOPE,	"Unbalanced Scope")

#define PGPMSG_NO_PUBKEY	1200
PGPMSG (PGPMSG_NO_PUBKEY,	"No public key available")
#define PGPMSG_NO_SECKEY	1201
PGPMSG (PGPMSG_NO_SECKEY, 	"No secret key available")

#define PGPMSG_COMMIT_INVALID	1250
PGPMSG (PGPMSG_COMMIT_INVALID,
	"Invalid response from commit: %d") 

#define PGPMSG_ESK_NODECRYPT	1300
PGPMSG (PGPMSG_ESK_NODECRYPT,	"Decryption unsuccessful")
#define PGPMSG_ESK_ADD_ERROR	1301
PGPMSG (PGPMSG_ESK_ADD_ERROR,	"Error adding ESK")
#define PGPMSG_ESK_BADTYPE	1302
PGPMSG (PGPMSG_ESK_BADTYPE,	"Bad ESK Type... skipping")
#define PGPMSG_ESK_BADPASS	1303
PGPMSG (PGPMSG_ESK_BADPASS,	"Passphrase incorrect.  Try Again.")

#define PGPMSG_SEPSIG_NOVERIFY	1350
PGPMSG (PGPMSG_SEPSIG_NOVERIFY,	"Separate signature unsuccessful")
#define PGPMSG_SEPSIG		1351
PGPMSG (PGPMSG_SEPSIG,		"This signature applies to another message")

#define PGPMSG_ANNOTATE_ERROR	1400
PGPMSG (PGPMSG_ANNOTATE_ERROR,	"Annotate returned an error")

#define PGPMSG_PKE_ENCRYPTED	1450
PGPMSG (PGPMSG_PKE_ENCRYPTED,
	"This message is public key encrypted using pke type %d")

#define PGPMSG_TRY_AGAIN	1500
PGPMSG (PGPMSG_TRY_AGAIN,	"Try again")

#define PGPMSG_DEVNULL_CREATE	1550
PGPMSG (PGPMSG_DEVNULL_CREATE,	"Cannot create Dev Null Module")

#define PGPMSG_ALLOC_HASH_BUFFER	1600
PGPMSG (PGPMSG_ALLOC_HASH_BUFFER,	"Failed to allocate hash buffer")
#define PGPMSG_ALLOC_HASH_CLONE	1601
PGPMSG (PGPMSG_ALLOC_HASH_CLONE,	"unable to clone hash; bailing")

#define PGPMSG_CONFIG_AMBIGUOUS		1650 
PGPMSG (PGPMSG_CONFIG_AMBIGUOUS,
	"line %u: ambigious key %.*s, matches %s and %s.")
#define PGPMSG_CONFIG_UNKNOWN_KEYWORD	1651
PGPMSG (PGPMSG_CONFIG_UNKNOWN_KEYWORD,	"line %u: unknown keyword %.*s")
#define PGPMSG_CONFIG_STRINGEND		1652
PGPMSG (PGPMSG_CONFIG_STRINGEND,	"line %u: unterminated string %.*s")
#define PGPMSG_CONFIG_MISSING_EQUAL	1653 
PGPMSG (PGPMSG_CONFIG_MISSING_EQUAL,	"line %u: Missing '=' after %.*s")
#define PGPMSG_CONFIG_MISSING_BOOLEAN	1654
PGPMSG (PGPMSG_CONFIG_MISSING_BOOLEAN,
	"line %u: Missing argument to boolean option %.*s")
#define PGPMSG_CONFIG_UNKNOWN_BOOLEAN	1655
PGPMSG (PGPMSG_CONFIG_UNKNOWN_BOOLEAN,
	"line %u: Unrecognized argument to boolean option %.*s: %.*s")
#define PGPMSG_CONFIG_UNKNOWN_INTEGER	1656
PGPMSG (PGPMSG_CONFIG_UNKNOWN_INTEGER,
	"line %u: Unrecognized argument to integer option %.*s: %.*s")
#define PGPMSG_CONFIG_MISSING_INTEGER	1657
PGPMSG (PGPMSG_CONFIG_MISSING_INTEGER,
	"line %u: Missing argument to integer option %.*s")
#define PGPMSG_CONFIG_INT_TOO_HIGH	1658
PGPMSG (PGPMSG_CONFIG_INT_TOO_HIGH,
	"line %u: Argument to %.*s is too high: %d, max %d")
#define PGPMSG_CONFIG_INT_TOO_LOW	1659
PGPMSG (PGPMSG_CONFIG_INT_TOO_LOW,
	"line %u: Argument to %.*s is too low: %d, min %d")
#define PGPMSG_CONFIG_INVALID_INTEGER	1660
PGPMSG (PGPMSG_CONFIG_INVALID_INTEGER,
	"line %u: Invalid argument to integer option %.*s: %.*s")
#define PGPMSG_CONFIG_STRING_TOO_LONG	1661
PGPMSG (PGPMSG_CONFIG_STRING_TOO_LONG,
	"line %u: Argument to %.*s is too long %d")
#define PGPMSG_CONFIG_INVALID_STRING	1662
PGPMSG (PGPMSG_CONFIG_INVALID_STRING,
	"line %u: Invalid argument to string option %.*s: %.*s")
#define PGPMSG_CONFIG_NO_FILE		1663
PGPMSG (PGPMSG_CONFIG_NO_FILE,	"Cannot open configuration file %s")
#define PGPMSG_CONFIG_LINE_TOO_LONG	1664
PGPMSG (PGPMSG_CONFIG_LINE_TOO_LONG,	"line %u: Line too long %s")
#define PGPMSG_CONFIG_BAD_CHAR		1665
PGPMSG (PGPMSG_CONFIG_BAD_CHAR,	"line %u: Bad Character\n%s\nOffset %d")
#define PGPMSG_CONFIG_MAX_ERRORS	1666
PGPMSG (PGPMSG_CONFIG_MAX_ERRORS,	"Processing halted after %d errors")
#define PGPMSG_CONFIG_NUM_ERRORS	1667
PGPMSG (PGPMSG_CONFIG_NUM_ERRORS,	"%d error(s) detected")

#endif /* Included_pgpMsg_h */