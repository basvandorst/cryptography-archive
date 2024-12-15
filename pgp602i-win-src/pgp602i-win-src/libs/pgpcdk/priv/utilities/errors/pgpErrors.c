/*____________________________________________________________________________
	pgpErrors.c
	
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpErrors.c,v 1.56.10.1 1998/11/12 03:23:16 heller Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpPubTypes.h"
#include "pgpErrors.h"
#include "pgpMem.h"


#include "pgpErrorsPriv.h"


typedef struct ErrorEntry
{
	PGPError const		number;
	const char * const	string;
} ErrorEntry;

#define EN(number, string)		{ number, string }


static const ErrorEntry sErrors[] =
{
/* later, if necessary, this table can be moved into its own file */
EN( kPGPError_NoErr,					"not an error"),
EN( kPGPError_BadParams,				"bad parameters"),
EN( kPGPError_OutOfMemory,			"out of memory"),
EN( kPGPError_BufferTooSmall,		"buffer too small"),

EN( kPGPError_FileNotFound,			"file not found"),
EN( kPGPError_CantOpenFile,			"can't open file"),
EN( kPGPError_FilePermissions,		"file permissions"),
EN( kPGPError_FileLocked,			"file locked"),
EN( kPGPError_DiskFull,				"disk full"),
EN( kPGPError_IllegalFileOp,			"illegal file operation"),
EN( kPGPError_FileOpFailed,			"file operation error"),
EN( kPGPError_ReadFailed,			"read failed"),
EN( kPGPError_WriteFailed,			"write failed"),
EN( kPGPError_EOF,					"end of file"),

EN( kPGPError_UserAbort,				"user cancelled"),
EN( kPGPError_UnknownRequest,		"unrecognized request"),
EN( kPGPError_LazyProgrammer,		"unknown error"),
EN( kPGPError_ItemNotFound,			"item not found"),
EN( kPGPError_ItemAlreadyExists,		"item already exists"),
EN( kPGPError_AssertFailed,			"assert failed"),
EN( kPGPError_BadMemAddress,			"bad memory address"),
EN( kPGPError_UnknownError,			"unknown error"),

EN( kPGPError_PrefNotFound,			"preference not found"),
EN( kPGPError_EndOfIteration,		"end of iteration"),
EN( kPGPError_ImproperInitialization,	"improper initialization"),
EN( kPGPError_CorruptData,			"corrupt data"),
EN( kPGPError_FeatureNotAvailable,	"feature not available"),


/* __________________________________________________________________ */
/* later, if necessary, this table can be moved into its own file */


EN( kPGPError_BadPassphrase,			"bad passphrase"),
EN( kPGPError_RedundantOptions,		"redundant options"),
EN( kPGPError_OptionNotFound,		"option not found"),
EN( kPGPError_CAPIUnsupportedKey,
						"key is not supported by Microsoft CAPI library"),

EN( kPGPError_KeyRevoked,				"key has been revoked"),
EN( kPGPError_KeyExpired,				"key has expired"),
EN( kPGPError_KeyDisabled,				"key has been disabled"),
EN( kPGPError_KeyInvalid,				"key validity is low or none"),
EN( kPGPError_KeyUnusableForEncryption,	"key can't be used for encryption"),
EN( kPGPError_KeyUnusableForSignature,	"key can't be used for signing"),
EN( kPGPError_OutputBufferTooSmall,		"space for output was too small"),
EN( kPGPError_InconsistentEncryptionAlgorithms,"No alg for all recips"),
EN( kPGPError_MissingPassphrase,			"needed passphrase is missing"),
EN( kPGPError_CombinedConventionalAndPublicEncryption,"conv & pk no go"),
EN( kPGPError_DetachedSignatureWithoutSigningKey,"Bad option combo"),
EN( kPGPError_DetachedSignatureWithEncryption,"Bad option combination"),
EN( kPGPError_NoInputOptions,			"wo input was specified"),
EN( kPGPError_MultipleInputOptions,		"only one input option allowed"),
EN( kPGPError_InputFile,					"problem opening input file"),
EN( kPGPError_NoOutputOptions,			"No output was specified"),
EN( kPGPError_MultipleOutputOptions,		"only one output option allowed"),
EN( kPGPError_MissingEventHandler,		"requested events but no handler"),
EN( kPGPError_MissingKeySet,				"need keyset for decoding"),
EN( kPGPError_DetachedSignatureFound,	"found detached sig, no callback"),
EN( kPGPError_NoDecryptionKeyFound,		"unable to decrypt, no key"),
EN( kPGPError_CorruptSessionKey,			"encrypted session key is bad"),
EN( kPGPError_SkipSection,				"user says to skip this section"),
EN( kPGPError_Interrupted,				"non-fatal interruption of job"),
EN( kPGPError_TooManyARRKs,				"exceeded ARR limit on keygen"),
EN( kPGPError_KeyUnusableForDecryption,	"key can't be used for decryption"),
EN( kPGPError_IncompatibleAPI,			"library version too old or too new"),

EN( kPGPError_NotMacBinary,						"not a MacBinary file"),
EN( kPGPError_NoMacBinaryTranslationAvailable,
		"MacBinary file can't be translated"),


EN( kPGPError_InconsistentFilterClasses,	"inconsisten filter classes"),
EN( kPGPError_UnsupportedLDAPFilter,		"this search unavailable for LDAP"),
EN( kPGPError_UnsupportedHKPFilter,		"this search unavailable for HTTP"),
EN( kPGPError_UnknownFilterType,			"unknown filter type"),
EN( kPGPError_InvalidFilterParameter,	"invalid filter parameter"),


/* old errors: */
EN( kPGPError_OutOfRings,				"internal keyring bits exhausted"),
EN( kPGPError_BadHashNumber,				"unknown hash number"),
EN( kPGPError_BadCipherNumber,			"unknown cipher number"),
EN( kPGPError_BadKeyLength,				"illegal key length for cipher"),
EN( kPGPError_SizeAdviseFailure,			"sizeAdvise promise not kept"),
EN( kPGPError_ConfigParseFailure,		"error parsing configuration"),
EN( kPGPError_ConfigParseFailureBadFunction,"invalid configuration function"),
EN( kPGPError_ConfigParseFailureBadOptions,"unknown configuration option"),
EN( kPGPError_KeyIsLocked,				"key requires passphrase to unlock"),
EN( kPGPError_CantDecrypt,				"cannot decrypt message"),
EN( kPGPError_BadSessionKeySize,
	"encrypted session key too short"),
EN( kPGPError_BadSessionKeyAlgorithm,
	"encrypted session key algorithm unknown"),
EN( kPGPError_UnknownSignatureType,		"unknown signature type"),
EN( kPGPError_BadSignatureSize,	"signature size too long or too short"),
EN( kPGPError_SignatureBitsWrong,	"wrong number of bits in signature"),
EN( kPGPError_ExtraDateOnSignature,		"invalid Extra Data for Signature"),
EN( kPGPError_SecretKeyNotFound,			"no secret key found"),
EN( kPGPError_AdditionalRecipientRequestKeyNotFound,
							"additional recipient request key not found"),
EN( kPGPError_InvalidCommit,				"invalid commit response"),
EN( kPGPError_CantHash,					"cannot hash message"),
EN( kPGPError_UnbalancedScope,			"unbalanced scope"),
EN( kPGPError_WrongScope,				"data sent in wrong scope"),
EN( kPGPError_PublicKeyTooSmall,			"public Key too small for data"),
EN( kPGPError_PublicKeyTooLarge,			"public Key too big for Library"),
EN( kPGPError_PublicKeyUnimplemented,"unimplemented public key operation"),
EN( kPGPError_FIFOReadError,				"incomplete read from Fifo"),
EN( kPGPError_RandomSeedTooSmall,		"not enough data in randseed file"),
EN( kPGPError_EnvPriorityTooLow,		"env var not set: priority too low"),
EN( kPGPError_UnknownCharMap,			"unknown charset"),
EN( kPGPError_AsciiParseIncomplete,		"ascii armor input incomplete"),



EN( kPGPError_TroubleKeySubKey,			"key matches subkey"),
EN( kPGPError_TroubleSigSubKey,			"signature by subkey"),
EN( kPGPError_TroubleBadTrust,			"trust packet malformed"),
EN( kPGPError_TroubleUnknownPacketByte,	"unknown packet PGPByte in keyring"),
EN( kPGPError_TroubleUnexpectedSubKey,	"unexpected subkey (before key),"),
EN( kPGPError_TroubleUnexpectedName,		"unexpected name (before key),"),
EN( kPGPError_TroubleUnexpectedSignature,"unexpected sig (before key),"),
EN( kPGPError_TroubleUnexpectedUnknown,
	"packet of unknown type in unexpected location (before key),"),
EN( kPGPError_TroubleUnexpectedTrust,	"unexpected trust packet"),
EN( kPGPError_TroubleKeyTooBig,			"key grossly oversized"),
EN( kPGPError_TroubleSecretKeyTooBig,	"secret key grossly oversized"),
EN( kPGPError_TroubleNameTooBig,			"name grossly oversized"),
EN( kPGPError_TroubleSignatureTooBig,	"sig grossly oversized"),
EN( kPGPError_TroubleUnknownTooBig,
	"packet of unknown type too large to handle"),
EN( kPGPError_TroubleDuplicateKeyID,		"duplicate KeyID, different keys"),
EN( kPGPError_TroubleDuplicateKey,		"duplicate key (in same keyring)"),
EN( kPGPError_TroubleDuplicateSecretKey,"duplicate secret (in same keyring)"),
EN( kPGPError_TroubleDuplicateName,		"duplicate name (in same keyring)"),
EN( kPGPError_TroubleDuplicateSignature,
	"duplicate signature (in same keyring)"),
EN( kPGPError_TroubleDuplicateUnknown,"duplicate unknown \"thing\" in keyring"),
EN( kPGPError_TroubleBareKey,			"key found with no names"),
EN( kPGPError_TroubleVersionBugPrev,	"bug introduced by legal_kludge"),
EN( kPGPError_TroubleVersionBugCur,		"bug introduced by legal_kludge"),
EN( kPGPError_TroubleOldSecretKey,		"passphrase is out of date"),
EN( kPGPError_TroubleNewSecretKey,		"passphrase is newer than another"),
EN( kPGPError_BadPacket,					"bad packet" ),
/*	
* Errors that can be encountered when parsing a key.
* If multiple errors apply, only the last is reported.
*
* Note: Obviously the modulus n = p*q must be odd, if p and q
* are both large primes, since all primes greater than 2 are odd.
* The exponent e must have a corresponding decryption exponent
* d such that e*d mod gcd(p-1,q-1), = 1.	Since p-1 and q-1 are both
* even, their gcd must also be even and thus e*d mod 2 = 1, implying
* that e mod 2 = 1.
* I.e. if e were even, the message would not be decryptable.
*/

/* kPGPError_KEY_LONG is a warning only */

EN( kPGPError_KEY_LONG,					"key packet has trailing junk"),
EN( kPGPError_KeyPacketTruncated,		"key packet truncated"),
EN( kPGPError_UnknownKeyVersion,			"key version unknown"),
EN( kPGPError_UnknownPublicKeyAlgorithm,	"key algorithm unknown"),
EN( kPGPError_MalformedKeyModulus,		"key modulus mis-formatted"),
EN( kPGPError_MalformedKeyExponent,		"key exponent mis-formatted"),
EN( kPGPError_RSAPublicModulusIsEven,	"RSA public modulus is even"),
EN( kPGPError_RSAPublicExponentIsEven,	"RSA public exponent is even"),
EN( kPGPError_MalformedKeyComponent,		"key component mis-formatted"),

/*
* Errors that can be encountered when parsing a signature.
* If multiple errors apply, only the last is reported.
*/


EN( kPGPError_SIG_LONG,					"signature packet has trailing junk"),
EN( kPGPError_TruncatedSignature,		"signature truncated"),
EN( kPGPError_MalformedSignatureInteger,	"signature integer mis-formatted"),
EN( kPGPError_UnknownSignatureAlgorithm,	"signature algorithm unknown"),
EN( kPGPError_ExtraSignatureMaterial,	"bad signature extra material (not 5)"),
EN( kPGPError_UnknownSignatureVersion,	"signature version unknown"),

/*	Mid-level key manipulation API error codes */
EN( kPGPError_OutOfEntropy,			"insufficient random bits"),
EN( kPGPError_ItemIsReadOnly,		"object is read-only"),
EN( kPGPError_InvalidProperty,		"invalid property name"),
EN( kPGPError_FileCorrupt,			"key database is corrupt"),
EN( kPGPError_UnknownVersion,		"unknown version"),
EN( kPGPError_DuplicateCert,			"duplicate certification"),
EN( kPGPError_DuplicateUserID,		"duplicate UserID"),
EN( kPGPError_CertifyingKeyDead,		"certifying key no longer valid"),
EN( kPGPError_ItemWasDeleted,		"Object has been deleted"),

/*	Certificate Server error codes */
EN( kPGPError_ServerInProgress,	"server operation in progress"),
EN( kPGPError_ServerOperationNotSupported,	"server operation not supported"),
EN( kPGPError_ServerInvalidProtocol,	"invalid server protocol"),
EN( kPGPError_ServerRequestFailed,	"the request was rejected by the server"),
EN( kPGPError_ServerOpen,	"server already open"),
EN( kPGPError_ServerNotOpen, "server not open"),
EN( kPGPError_ServerKeyAlreadyExists,
								"the exact key already exists on the server"),
EN( kPGPError_ServerNotInitialized,	"server not initialized"),
EN( kPGPError_ServerPartialAddFailure,
								"some keys could not be added to the server"),
EN( kPGPError_ServerCorruptKeyBlock,	"corrupt keyblock"),
EN( kPGPError_ServerUnknownResponse, "unknown response from server"),
EN( kPGPError_ServerTimedOut,	"server request timed out"),
EN( kPGPError_ServerOpenFailed,		"server open failed"),
EN( kPGPError_ServerAuthorizationRequired, 
								"authorization required for this operation"),
EN( kPGPError_ServerAuthorizationFailed, 
									"authorization failed for this operation"),
EN( kPGPError_ServerSearchFailed,	"server search failed"),
EN( kPGPError_ServerPartialSearchResults,	"server returned partial results"),
EN( kPGPError_ServerBadKeysInSearchResults,
								"the server returned 1 or more corrupt keys"),
EN( kPGPError_ServerKeyFailedPolicy, "the key failed the server policy check"),
EN( kPGPError_ServerOperationRequiresTLS, "the operation requires TLS"),
EN( kPGPError_ServerNoStaticStorage,
		"server static storage not allocated for thread"),

/* TLS error codes */
EN( kPGPError_TLSUnexpectedClose,
								"the TLS connection was unexpectedly shutdown"),
EN( kPGPError_TLSProtocolViolation,
					"the remote system violated the TLS connection protocol"),
EN( kPGPError_TLSVersionUnsupported,
					"the TLS version of the remote system is not compatible"),
EN( kPGPError_TLSWrongState,
					"the remote system violated the TLS handshake sequence"),
EN( kPGPError_TLSAlertReceived, "the remote system aborted the TLS connection"),
EN( kPGPError_TLSKeyUnusable,
					"the selected key is not capable of TLS authentication."),
EN( kPGPError_TLSNoCommonCipher,
			"the encryption suggested by the remote TLS system is not usable"),
EN( kPGPError_TLSWouldBlock,
			"TLS would block"),
EN( kPGPError_TLSRcvdHandshakeRequest,
			"the remote system requested a renegotiation of the key schedules"),

/* Sockets error codes */
EN( kPGPError_SocketsNetworkDown,	"network down"),
EN( kPGPError_SocketsNotInitialized, "sockets not initialized"),
EN( kPGPError_SocketsInProgress,	"sockets call already in progress"),
EN( kPGPError_SocketsNotConnected,	"socket is not connected"),
EN( kPGPError_SocketsNotBound, "socket has not been bound"),
EN( kPGPError_SocketsOperationNotSupported,
									"operation not supported on this socket"),
EN( kPGPError_SocketsProtocolNotSupported,
									"protocol not supported by sockets"),
EN( kPGPError_SocketsAddressFamilyNotSupported,
									"address family not supported by sockets"),
EN( kPGPError_SocketsNotASocket,	"not a socket"),
EN( kPGPError_SocketsAddressInUse,	"address already in use by another socket"),
EN( kPGPError_SocketsBufferOverflow,	"buffer overflow"),
EN( kPGPError_SocketsListenQueueFull,	"listen queue is full"),
EN( kPGPError_SocketsAddressNotAvailable,	"address not available to socket"),
EN( kPGPError_SocketsAlreadyConnected,	"socket is already connected"),
EN( kPGPError_SocketsTimedOut,	"socket timed out during operation"),
EN( kPGPError_SocketsHostNotFound,	"host not found"),
EN( kPGPError_SocketsDomainServerError,	"error in domain name system"),
EN( kPGPError_SocketsNoStaticStorage,
		"sockets static storage not allocated for thread")
};
#define kPGPErrors_NumErrorTableEntries		\
	( sizeof( sErrors ) / sizeof( sErrors[ 0 ] ) )

#undef EN


	
	PGPError 
PGPGetErrorString(
	PGPError	theError,
	PGPSize		bufferSize,
	char *		theString )
{
	PGPUInt32		idx;
	char			temp[ 256 ];
	char const *	errStr	= NULL;
	PGPSize			len	= 0;
	PGPBoolean		bufferBigEnough	= FALSE;
	
	PGPValidateParam( bufferSize >= 1 );
	PGPValidatePtr( theString );
	*theString	= '\0';
	
	for( idx = 0; idx < kPGPErrors_NumErrorTableEntries; ++idx )
	{
		const ErrorEntry *	entry;
		
		entry	= &sErrors[ idx ];
		if ( entry->number == theError )
		{
			errStr	= entry->string;
			break;
		}
	}
	
	if ( IsNull( errStr ) )
	{
		/* Produce something for missing errors */
		sprintf( temp, "PGPError #%ld", (long)theError );
		errStr	= temp;
	}
	
	len	= strlen( errStr );
	bufferBigEnough	= ( len + 1 <= bufferSize );
	if ( bufferBigEnough )
	{
		strcpy( theString, errStr );
	}
	else
	{
		pgpCopyMemory( errStr, theString, bufferSize - 1 );
		theString[ bufferSize - 1 ]	= '\0';
	}
	
	return( bufferBigEnough	? kPGPError_NoErr : kPGPError_BufferTooSmall );
}



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
