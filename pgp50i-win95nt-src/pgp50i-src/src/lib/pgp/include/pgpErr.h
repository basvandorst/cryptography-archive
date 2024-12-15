/*
 * pgpErr.h -- This file contains the list of PGP errno values (i.e., the
 * the error values that are returned from functions returning
 * an error)
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * The errors are grouped into blocks of codes for specific functions, and
 * they should all be negative!
 *
 * Written by:	Colin Plumb and Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Definition Header.
 *
 * $Id: pgpErr.h,v 1.21.2.13 1997/06/24 13:26:06 quark Exp $
 */

#ifndef PGPERR_H
#define PGPERR_H

/*
* Code to translate error codes to strings can define this macro and
* #include this file to perform some sort of lookup table initialization.
* Please update this in the obvious way when you change this file.
*/
#ifndef PGPERR	/* [ */
#define PGPERR(num, string) /*nothing*/

/* Only include declarations if we're not doing preprocessor magic */

#ifdef __cplusplus
extern "C" {
#endif

char const PGPExport *pgperrString(int error);

typedef int PGPError;	/* XXX Most places still use int directly, by the way */

#ifdef __cplusplus
}
#endif

#endif	/* ] PGPERR */

/*
* All messages with a negative error code should be printed. Some positive
* "error codes" are used as severity levels to control masking of inessential
* messages.
*/
	#define PGPERR_VERBOSE_0		0
	#define PGPERR_VERBOSE_1		1
	#define PGPERR_VERBOSE_2		2
	#define PGPERR_VERBOSE_3		3

	#define PGPERR_OK				0
	PGPERR (PGPERR_OK,	 				"No errors")

/*
* Avoid using this code - if it appears, it means that the programmer was
* too lazy to find the right one. It's usually written in the source as
* just plain "-1".
*/
	#define PGPERR_GENERIC	 			-1
	PGPERR (PGPERR_GENERIC,					"Generic error (should be changed)")

	#define PGPERR_NOMEM				-2
	PGPERR (PGPERR_NOMEM,					"Out of Memory")

	#define PGPERR_BADPARAM				-3
	PGPERR (PGPERR_BADPARAM,					"Invalid Parameter")

	#define PGPERR_NO_FILE	 			-5
	PGPERR (PGPERR_NO_FILE,					"Cannot open file")

	#define PGPERR_NO_KEYBITS			-6
	PGPERR (PGPERR_NO_KEYBITS,					"Internal keyring bits exhausted")

	#define PGPERR_BAD_HASHNUM			-10
	PGPERR (PGPERR_BAD_HASHNUM,					"Unknown hash number")

	#define PGPERR_BAD_CIPHERNUM	-15
	PGPERR (PGPERR_BAD_CIPHERNUM,					"Unknown cipher number")
	#define PGPERR_BAD_KEYLEN			-16
	PGPERR (PGPERR_BAD_KEYLEN, 					"Illegal key length for cipher")

	#define PGPERR_SIZEADVISE			-20
	PGPERR (PGPERR_SIZEADVISE, 					"SizeAdvise promise not kept")

	#define PGPERR_CONFIG				-100
	PGPERR (PGPERR_CONFIG,	 					"Error parsing configuration")
#define PGPERR_CONFIG_BADFUNC	-101
PGPERR (PGPERR_CONFIG_BADFUNC,		"Invalid configuration function")
#define PGPERR_CONFIG_BADOPT	-102
PGPERR (PGPERR_CONFIG_BADOPT,		"Unknown configuration option")

#define PGPERR_STRING_NOT_FOUND -105
PGPERR (PGPERR_STRING_NOT_FOUND,        "Requested string not found")
#define PGPERR_STRING_NOT_IN_LANGUAGE   -106
PGPERR (PGPERR_STRING_NOT_IN_LANGUAGE,  "Requested string not in language")

#define PGPERR_KEY_ISLOCKED	-110
PGPERR (PGPERR_KEY_ISLOCKED,		"Key requires passphrase to unlock")
#define PGPERR_KEY_UNUNLOCKABLE	-111
PGPERR (PGPERR_KEY_UNUNLOCKABLE,	"Key requires passphrase each time")

	#define PGPERR_SIG_ERROR	-120
	PGPERR (PGPERR_SIG_ERROR,					"Error while processing signature")
	#define PGPERR_ADDSIG_ERROR	-121
	PGPERR (PGPERR_ADDSIG_ERROR,					"Cannot add signature")

	#define PGPERR_CANNOT_DECRYPT	-125
	PGPERR (PGPERR_CANNOT_DECRYPT,	 				"Cannot decrypt message")
	#define PGPERR_ADDESK_ERROR	-126
	PGPERR (PGPERR_ADDESK_ERROR,					"Cannot add encrypted session key")

	#define PGPERR_UNK_STRING2KEY	-127
	PGPERR (PGPERR_UNK_STRING2KEY,	 				"Don't know how to convert pass"
			 		 				" phrase to key")
	#define PGPERR_BAD_STRING2KEY	-128
	PGPERR (PGPERR_BAD_STRING2KEY,	 				"Invalid conversion from pass"
			 		 				" phrase to key")

	#define PGPERR_ESK_BADTYPE	 -130
	PGPERR (PGPERR_ESK_BADTYPE,			"Unknown encrypted session key type")
	#define PGPERR_ESK_TOOSHORT	-131
	PGPERR (PGPERR_ESK_TOOSHORT,			"Encrypted session key too short")
	#define PGPERR_ESK_TOOLONG	 -132
	PGPERR (PGPERR_ESK_TOOLONG,			"Encrypted session key too long")
	#define PGPERR_ESK_BADVERSION	-133
	PGPERR (PGPERR_ESK_BADVERSION,	 		"Encrypted session key version"
			 		 		" unknown")
	#define PGPERR_ESK_BADALGORITHM	-134
	PGPERR (PGPERR_ESK_BADALGORITHM,		"Encrypted session key algorithm"
			 		 		" unknown")
	#define PGPERR_ESK_BITSWRONG	-135
	PGPERR (PGPERR_ESK_BITSWRONG,			"Wrong number of bits in ESK")
	#define PGPERR_ESK_NOKEY	-136
	PGPERR (PGPERR_ESK_NOKEY,	"Can't find key to decrypt session key")
	#define PGPERR_ESK_NODECRYPT	-137
	PGPERR (PGPERR_ESK_NODECRYPT,			"Can't decrypt this session key")
	#define PGPERR_ESK_BADPASS		-138
	PGPERR (PGPERR_ESK_BADPASS,			"Passphrase incorrect")

	#define PGPERR_SIG_BADTYPE		-140
	PGPERR (PGPERR_SIG_BADTYPE,			"Unknown signature type")
	#define PGPERR_SIG_TOOSHORT		-141
	PGPERR (PGPERR_SIG_TOOSHORT,			"Signature too short")
	#define PGPERR_SIG_TOOLONG	 -142
	PGPERR (PGPERR_SIG_TOOLONG,			"Signature too long")
	#define PGPERR_SIG_BADVERSION	-143
	PGPERR (PGPERR_SIG_BADVERSION,	 		"Signature version unknown")
	#define PGPERR_SIG_BADALGORITHM	-144
	PGPERR (PGPERR_SIG_BADALGORITHM,		"Signature algorithm unknown")
	#define PGPERR_SIG_BITSWRONG	-145
	PGPERR (PGPERR_SIG_BITSWRONG,			"Wrong number of bits in signature")
	#define PGPERR_SIG_NOKEY	-146
	PGPERR (PGPERR_SIG_NOKEY,	"Can't find necessary key to check sig")
	#define PGPERR_SIG_BADEXTRA	-147
	PGPERR (PGPERR_SIG_BADEXTRA,			"Invalid Extra Data for Signature")

	#define PGPERR_NO_PUBKEY	-150
	PGPERR (PGPERR_NO_PUBKEY,			"No public key found")
	#define PGPERR_NO_SECKEY	-151
	PGPERR (PGPERR_NO_SECKEY,			"No secret key found")
	#define PGPERR_UNKNOWN_KEYID	-152
	PGPERR (PGPERR_UNKNOWN_KEYID,			"No matching keyid found")
	#define PGPERR_NO_RECOVERYKEY	-153
	PGPERR (PGPERR_NO_RECOVERYKEY,	 		"Requested message recovery key"
			 		 		" not found")

	#define PGPERR_COMMIT_INVALID	-155
	PGPERR (PGPERR_COMMIT_INVALID,	 		"Invalid commit response")
	#define PGPERR_CANNOT_HASH	 -156
	PGPERR (PGPERR_CANNOT_HASH,			"Cannot hash message")

	#define PGPERR_UNBALANCED_SCOPE	-160
	PGPERR (PGPERR_UNBALANCED_SCOPE,		"Unbalanced scope")
	#define PGPERR_WRONG_SCOPE	 -161
	PGPERR (PGPERR_WRONG_SCOPE,			"Data sent in wrong scope")

	#define PGPERR_UI_INVALID		-165
	PGPERR (PGPERR_UI_INVALID,				"Invalid UI Callback Object")
	#define PGPERR_CB_INVALID		-166
	PGPERR (PGPERR_CB_INVALID,				"Invalid Parser Callback")
	#define PGPERR_INTERRUPTED		-167
	PGPERR (PGPERR_INTERRUPTED,				"Interrupted encrypt/decrypt"
			 					" operation")

	#define PGPERR_PUBKEY_TOOSMALL	 -170
	PGPERR (PGPERR_PUBKEY_TOOSMALL,				"Public Key too small for data")
	#define PGPERR_PUBKEY_TOOBIG	-171
	PGPERR (PGPERR_PUBKEY_TOOBIG,	"Public key is too big for this version")
	#define PGPERR_PUBKEY_UNIMP	-172
	PGPERR (PGPERR_PUBKEY_UNIMP,				"Unimplemented public key operation")

	#define PGPERR_RSA_CORRUPT		-175
	PGPERR (PGPERR_RSA_CORRUPT,				"Corrupt data decrypting RSA block")
	#define PGPERR_PK_CORRUPT		-176
	PGPERR (PGPERR_PK_CORRUPT,				"Corrupt data decrypting public"
			 					" key block")

	#define PGPERR_CMD_TOOBIG		-180
	PGPERR (PGPERR_CMD_TOOBIG,				"Command to Buffer too big")
	#define PGPERR_FIFO_READ		-181
	PGPERR (PGPERR_FIFO_READ,				"Incomplete read from Fifo")

	#define PGPERR_VRFYSIG_WRITE	-185
	PGPERR (PGPERR_VRFYSIG_WRITE,				"Data illegally written into"
			 		 			" signature pipe")
	#define PGPERR_VRFYSIG_BADANN	-186
	PGPERR (PGPERR_VRFYSIG_BADANN,	 			"Invalid annotation to signature"
			 		 			" verifier")

	#define PGPERR_ADDHDR_FLUSH	-190
	PGPERR (PGPERR_ADDHDR_FLUSH,				"Cannot flush buffer until size"
			 		 			" is known")

	#define PGPERR_JOIN_BADANN	 -195
	PGPERR (PGPERR_JOIN_BADANN,				"Invalid annotation to join module")

	#define PGPERR_RANDSEED_TOOSMALL -200
	PGPERR (PGPERR_RANDSEED_TOOSMALL,			"Not enough data in randseed file")

	#define PGPERR_ENV_LOWPRI	-205
	PGPERR (PGPERR_ENV_LOWPRI,	 			"Env Var not set: priority too low")
	#define PGPERR_ENV_BADVAR	-206
	PGPERR (PGPERR_ENV_BADVAR,	 			"Invalid environment variable")

	#define PGPERR_CHARMAP_UNKNOWN	 -210
	PGPERR (PGPERR_CHARMAP_UNKNOWN,				"Unknown Charset")

	#define PGPERR_FILE_PERMISSIONS	-213
	PGPERR (PGPERR_FILE_PERMISSIONS,			"Unsufficient file permissions")

	#define PGPERR_FILE_WRITELOCKED	-214
	PGPERR (PGPERR_FILE_WRITELOCKED,			"File already open for writing")

	#define PGPERR_FILE_BADOP		-215
	PGPERR (PGPERR_FILE_BADOP,				"Invalid PgpFile Operation")

	#define PGPERR_FILE_OPFAIL		-216
	PGPERR (PGPERR_FILE_OPFAIL,				"PgpFile Operation Failed")

	#define PGPERR_IMMUTABLE		-217
	PGPERR (PGPERR_IMMUTABLE,				"Attempt to change an"
			 					" immutable object")

	#define PGPERR_PARSEASC_INCOMPLETE -220
	PGPERR (PGPERR_PARSEASC_INCOMPLETE,			"Ascii Armor Input Incomplete")

	#define PGPERR_PARSEASC_BADINPUT -221
	PGPERR (PGPERR_PARSEASC_BADINPUT,			"PGP text input is corrupted")

	#define PGPERR_FILEFIFO_SEEK	-230
	PGPERR (PGPERR_FILEFIFO_SEEK,				"Temp-File Seek Error")

	#define PGPERR_FILEFIFO_WRITE	-231
	PGPERR (PGPERR_FILEFIFO_WRITE,	 			"Temp-File Write Error")

	#define PGPERR_FILEFIFO_READ	-232
	PGPERR (PGPERR_FILEFIFO_READ,				"Temp-File Read Error")

#define PGPERR_FILEIO_BADPKT	-235
PGPERR (PGPERR_FILEIO_BADPKT,	         "Corrupted or bad packet in"
                                         " PGP data file")

#define PGPERR_SYSTEM_PGPK      -240
PGPERR (PGPERR_SYSTEM_PGPK,             "Error Executing PGPK Program")
#define PGPERR_UNKNOWN_ARGUMENT	-241
PGPERR (PGPERR_UNKNOWN_ARGUMENT,	"Unknown argument")
#define PGPERR_MISSING_ARGUMENT -242
PGPERR (PGPERR_MISSING_ARGUMENT,	"Missing argument")

/* These errors occur only in key management, and have extra info attached. */
#define PGPERR_KEYIO_READING   -300
PGPERR (PGPERR_KEYIO_READING,          "I/O error reading keyring")

	#define PGPERR_KEYIO_WRITING	-301
	PGPERR (PGPERR_KEYIO_WRITING,				"I/O error writing keyring")

	#define PGPERR_KEYIO_FTELL	 -302
	PGPERR (PGPERR_KEYIO_FTELL,				"I/O error finding keyring position")

	#define PGPERR_KEYIO_SEEKING	-303
	PGPERR (PGPERR_KEYIO_SEEKING,				"I/O error seeking keyring")

	#define PGPERR_KEYIO_OPENING	-304
	PGPERR (PGPERR_KEYIO_OPENING,				"I/O error opening keyring")

	#define PGPERR_KEYIO_CLOSING	-305
	PGPERR (PGPERR_KEYIO_CLOSING,				"I/O error closing keyring")

	#define PGPERR_KEYIO_FLUSHING	-306
	PGPERR (PGPERR_KEYIO_FLUSHING,	 			"I/O error flushing keyring")

/*
* The following errors indicate that someone modified a file while it
* was open for access. These can also be found in trouble logs.
*/
/* Did a seek to a known packet, encountered EOF there! */
#define PGPERR_KEYIO_EOF	-310
PGPERR (PGPERR_KEYIO_EOF,		 "Unexpected EOF fetching key packet")

/*
* Did a seek to a known packet, found something different there!
* If this happens after a keyring has been opened, it means that
* the keyring has been changed out from under PGP!
*/
	#define PGPERR_KEYIO_BADPKT	-311
	PGPERR (PGPERR_KEYIO_BADPKT,			"Bad data found where key"
			 		 		" packet expected")

	#define PGPERR_KEYIO_BADFILE	-312
	PGPERR (PGPERR_KEYIO_BADFILE,			"Not a keyring file")

/*
* The following errors are warnings found in RingFile trouble logs only.
* The only other possibilities found in trouble logs are
* - PGPERR_KEYIO_READING
* - PGPERR_KEYIO_FTELL
* - PGPERR_KEYIO_EOF
* - PGPERR_KEYIO_BADPKT
* - PGPERR_NOMEM
*/
	#define PGPERR_TROUBLE_MAX		-318
	#define PGPERR_TROUBLE_MIN		-349

	#define PGPERR_TROUBLE_KEYSUBKEY -318
	PGPERR (PGPERR_TROUBLE_KEYSUBKEY,		"Key matches subkey")
	#define PGPERR_TROUBLE_SIGSUBKEY -319
	PGPERR (PGPERR_TROUBLE_SIGSUBKEY,		"Signature by subkey")
	#define PGPERR_TROUBLE_BADTRUST	-320
	PGPERR (PGPERR_TROUBLE_BADTRUST,		"Trust packet malformed")
	#define PGPERR_TROUBLE_UNKPKTBYTE -321
	PGPERR (PGPERR_TROUBLE_UNKPKTBYTE,		"Unknown packet byte in keyring")
	#define PGPERR_TROUBLE_UNXSUBKEY -322
	PGPERR (PGPERR_TROUBLE_UNXSUBKEY,		"Unexpected subkey (before key)")
	#define PGPERR_TROUBLE_UNXNAME	 -323
	PGPERR (PGPERR_TROUBLE_UNXNAME,			"Unexpected name (before key)")
	#define PGPERR_TROUBLE_UNXSIG	-324
	PGPERR (PGPERR_TROUBLE_UNXSIG,	 		"Unexpected sig (before key)")
	#define PGPERR_TROUBLE_UNXUNK	-325
	PGPERR (PGPERR_TROUBLE_UNXUNK,	 		"Packet of unknown type in unexpected"
			 		 		" location (before key)")
	#define PGPERR_TROUBLE_UNXTRUST	-326
	PGPERR (PGPERR_TROUBLE_UNXTRUST,		"Unexpected trust packet")
	#define PGPERR_TROUBLE_KEY2BIG	 -327
	PGPERR (PGPERR_TROUBLE_KEY2BIG,			"Key grossly oversized")
	#define PGPERR_TROUBLE_SEC2BIG	 -328
	PGPERR (PGPERR_TROUBLE_SEC2BIG,			"Secret key grossly oversized")
	#define PGPERR_TROUBLE_NAME2BIG	-329
	PGPERR (PGPERR_TROUBLE_NAME2BIG,		"Name grossly oversized")
	#define PGPERR_TROUBLE_SIG2BIG	 -330
	PGPERR (PGPERR_TROUBLE_SIG2BIG,			"Sig grossly oversized")
	#define PGPERR_TROUBLE_UNK2BIG	 -331
	PGPERR (PGPERR_TROUBLE_UNK2BIG,			"Packet of unknown type too large"
			 		 		" to handle")
	#define PGPERR_TROUBLE_DUPKEYID	-332
	PGPERR (PGPERR_TROUBLE_DUPKEYID, 		"Duplicate KeyID, different keys")
	#define PGPERR_TROUBLE_DUPKEY	-333
	PGPERR (PGPERR_TROUBLE_DUPKEY,	 		"Duplicate key (in same keyring)")
	#define PGPERR_TROUBLE_DUPSEC	-334
	PGPERR (PGPERR_TROUBLE_DUPSEC,	 		"Duplicate secret (in same keyring)")
	#define PGPERR_TROUBLE_DUPNAME	 -335
	PGPERR (PGPERR_TROUBLE_DUPNAME,			"Duplicate name (in same keyring)")
	#define PGPERR_TROUBLE_DUPSIG	-336
	PGPERR (PGPERR_TROUBLE_DUPSIG,	 "Duplicate signature (in same keyring)")
	#define PGPERR_TROUBLE_DUPUNK	-337
	PGPERR (PGPERR_TROUBLE_DUPUNK,	 "Duplicate unknown \"thing\" in keyring")
	#define PGPERR_TROUBLE_BAREKEY	 -338
	PGPERR (PGPERR_TROUBLE_BAREKEY,			"Key found with no names")
	#define PGPERR_TROUBLE_VERSION_BUG_PREV	-339
	PGPERR (PGPERR_TROUBLE_VERSION_BUG_PREV, "Bug introduced by legal_kludge")
	#define PGPERR_TROUBLE_VERSION_BUG_CUR	 -340
	PGPERR (PGPERR_TROUBLE_VERSION_BUG_CUR,	"Bug introduced by legal_kludge")
	#define PGPERR_TROUBLE_OLDSEC	-341
	PGPERR (PGPERR_TROUBLE_OLDSEC,	 		"Passphrase is out of date")
	#define PGPERR_TROUBLE_NEWSEC	-342
	PGPERR (PGPERR_TROUBLE_NEWSEC,	 		"Passphrase is newer than another")
/*
* Errors that can be encountered when parsing a key.
* If multiple errors apply, only the last is reported.
*
* Note: Obviously the modulus n = p*q must be odd, if p and q
* are both large primes, since all primes greater than 2 are odd.
* The exponent e must have a corresponding decryption exponent
* d such that e*d mod gcd(p-1,q-1) = 1. Since p-1 and q-1 are both
* even, their gcd must also be even and thus e*d mod 2 = 1, implying
* that e mod 2 = 1.
* I.e. if e were even, the message would not be decryptable.
*
* ANY ERRORS NOT IN THE RANGE PGPERR_KEY_MAX..PGPERR_KEY_MIN, IF ENCOUNTERED
* DURING A MAINTENANCE PASS, HAVE THE EFFECT OF ABORTING THE PASS. ANY NEW
* KEY-ASSOCIATED ERROR MUST BE ADDED TO THIS RANGE IF THE MAINTENANCE PASS
* IS TO JUST SKIP THAT KEY AND NOT ABORT THE WHOLE PASS.
*/

#define PGPERR_KEY_MAX	-347
#define PGPERR_KEY_MIN	-359

#define PGPERR_KEY_NO_RSA_ENCRYPT -347
PGPERR (PGPERR_KEY_NO_RSA_ENCRYPT, "No RSA Encryption/Signature support")
#define PGPERR_KEY_NO_RSA_DECRYPT -348
PGPERR (PGPERR_KEY_NO_RSA_DECRYPT, "No RSA Decryption/Verification support")
#define PGPERR_KEY_NO_RSA	-349
PGPERR (PGPERR_KEY_NO_RSA,			"No RSA key support")

/* PGPERR_KEY_LONG is a warning only */
#define PGPERR_KEY_LONG			-350
PGPERR (PGPERR_KEY_LONG,			"Key packet has trailing junk")
#define PGPERR_KEY_SHORT		-351
PGPERR (PGPERR_KEY_SHORT,			 "Key packet truncated")
#define PGPERR_KEY_VERSION		-352
PGPERR (PGPERR_KEY_VERSION,			"Key version unknown")
#define PGPERR_KEY_PKALG		-353
	PGPERR (PGPERR_KEY_PKALG,				"Key algorithm unknown")
	#define PGPERR_KEY_MODMPI		-354
	PGPERR (PGPERR_KEY_MODMPI,				"Key modulus mis-formatted")
	#define PGPERR_KEY_EXPMPI		-355
	PGPERR (PGPERR_KEY_EXPMPI,				"Key exponent mis-formatted")
	#define PGPERR_KEY_MODEVEN		-356
	PGPERR (PGPERR_KEY_MODEVEN,				"RSA public modulus is even")
	#define PGPERR_KEY_EXPEVEN		-357
	PGPERR (PGPERR_KEY_EXPEVEN,				"RSA public exponent is even")
	#define PGPERR_KEY_MPI	 		-358
	PGPERR (PGPERR_KEY_MPI,					"Key component mis-formatted")
	#define PGPERR_KEY_UNSUPP		-359
	PGPERR (PGPERR_KEY_UNSUPP,		"Key is not supported by this version of PGP")

/*
* Errors that can be encountered when parsing a signature.
* If multiple errors apply, only the last is reported.
*/
#define PGPERR_SIG_MAX	-360
#define PGPERR_SIG_MIN	-369

	#define PGPERR_SIG_LONG			-360
	PGPERR (PGPERR_SIG_LONG,				"Signature packet has trailing junk")
	#define PGPERR_SIG_SHORT		-361
	PGPERR (PGPERR_SIG_SHORT,				"Signature truncated")
	#define PGPERR_SIG_MPI	 		-362
	PGPERR (PGPERR_SIG_MPI,					"Signature integer mis-formatted")
	#define PGPERR_SIG_PKALG		-363
	PGPERR (PGPERR_SIG_PKALG,				"Signature algorithm unknown")
	#define PGPERR_SIG_EXTRALEN		-364
	PGPERR (PGPERR_SIG_EXTRALEN,				"Bad signature extra material (not 5)")
	#define PGPERR_SIG_VERSION	 -355
	PGPERR (PGPERR_SIG_VERSION,				"Signature version unknown")


/*	Mid-level key manipulation API error codes */

#define PGPERR_KEYDB_MAX -370
#define PGPERR_KEYDB_MIN -383

	#define PGPERR_KEYDB_BADPASSPHRASE			-370
	PGPERR (PGPERR_KEYDB_BADPASSPHRASE,				"Bad passphrase")
	#define PGPERR_KEYDB_KEYDBREADONLY		-371
	PGPERR (PGPERR_KEYDB_KEYDBREADONLY,				"Key database is read-only")
	#define PGPERR_KEYDB_NEEDMOREBITS		-372
	PGPERR (PGPERR_KEYDB_NEEDMOREBITS,				"Insufficient random bits")
	#define PGPERR_KEYDB_OBJECTREADONLY		-373
	PGPERR (PGPERR_KEYDB_OBJECTREADONLY,				"Object is read-only")
	#define PGPERR_KEYDB_INVALIDPROPERTY	-374
	PGPERR (PGPERR_KEYDB_INVALIDPROPERTY,				"Invalid property name")
	#define PGPERR_KEYDB_BUFFERTOOSHORT	-375
	PGPERR (PGPERR_KEYDB_BUFFERTOOSHORT,				"Buffer too short")
	#define PGPERR_KEYDB_CORRUPT			-376
	PGPERR (PGPERR_KEYDB_CORRUPT,					"Key database is corrupt")
	#define PGPERR_KEYDB_VERSIONTOONEW		-377
	PGPERR (PGPERR_KEYDB_VERSIONTOONEW,				"Data is too new to be read"
			 		 				" by this version")
	#define PGPERR_KEYDB_IOERROR			-378
	PGPERR (PGPERR_KEYDB_IOERROR,				"Input/output error")
	#define PGPERR_KEYDB_VALUETOOLONG	-379
	PGPERR (PGPERR_KEYDB_VALUETOOLONG,	 		"Value too long")
	#define PGPERR_KEYDB_DUPLICATE_CERT	-380
	PGPERR (PGPERR_KEYDB_DUPLICATE_CERT,			"Duplicate certification")
	#define PGPERR_KEYDB_DUPLICATE_USERID	-381
	PGPERR (PGPERR_KEYDB_DUPLICATE_USERID,	"Duplicate UserID")
	#define PGPERR_KEYDB_CERTIFYINGKEY_DEAD	-382
	PGPERR (PGPERR_KEYDB_CERTIFYINGKEY_DEAD, 		"Certifying key no longer"
			 		 			 " valid")
	#define PGPERR_KEYDB_OBJECT_DELETED	-383
	PGPERR (PGPERR_KEYDB_OBJECT_DELETED,	"Object has been deleted")

#endif /* PGPERR_H */
