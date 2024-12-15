/*
 * pgpMSRSAGlue.c - Interface to Microsoft CryptoAPI for RSA support
 *
 * This is an alternative to pgpRSAGlue.c, and should be used if that
 * module cannot be used for legal reasons.  It is only usable on
 * platforms which include Microsoft CAPI support.
 *
 * This code only accepts PKCS-style padding.  Very old versions of PGP
 * used a different padding style, which will not be compatible with
 * this module.
 *
 * $Id: pgpMSRSAGlue.c,v 1.10 1998/08/10 19:30:20 hal Exp $
 */
 
#include "pgpSDKBuildFlags.h"
/*

 * This entire source file is under the control of the following conditional:
 */

#if PGP_USECAPIFORRSA	/* [ */


#include "pgpConfig.h"

#include "pgpDebug.h"
#include "pgpPubKey.h"
#include "pgpRSAGlue.h"
#include "pgpKeyMisc.h"
#include "bn.h"
#include "pgpErrors.h"
#include "pgpRandomX9_17.h"
#include "pgpMem.h"
#include "pgpHashPriv.h"
#include "pgpMSRSAGlue.h"

/* Early versions of wincrypt.h are missing these */
#ifndef MS_ENH_PROV
#define MS_ENH_PROV_A       "Microsoft Enhanced Cryptographic Provider v1.0"
#define MS_ENH_PROV_W       L"Microsoft Enhanced Cryptographic Provider v1.0"
#ifdef UNICODE
#define MS_ENH_PROV         MS_ENH_PROV_W
#else
#define MS_ENH_PROV         MS_ENH_PROV_A
#endif
#endif /* MS_ENH_PROV */

#ifndef CALG_3DES
#define CALG_3DES 0x00006603
#endif /* CALG_3DES */


/* Shorthand for accessing CAPI externs */
#define ex pgp_CAPI_externs

/* Our key container name, chosen not to conflict with a user name */
#define PGPCONTAINER	"PGPreservedContainer"

/* Magic values for blob headers */
#define RSAPUBMAGIC		0x31415352
#define RSAPRIVMAGIC	0x32415352
#define BVERSION		2

/* Byte packing macros, little endian */
#define B4(val) (val)&0xff, ((val)>>8)&0xff, ((val)>>16)&0xff, ((val)>>24)&0xff
#define B2(val) (val)&0xff, ((val)>>8)&0xff

/* Round n up to be a multiple of r */
#define MULOF(n,r) ((((n)+(r)-1)/(r))*(r))


/* Blob definitions */

/* Blob header for all blobs */
typedef struct _BLOBHEADER
{
	BYTE bType;
	BYTE bVersion;
	WORD reserved;
	ALG_ID aiKeyAlg;
} BLOBHEADER;


/* Public key blob header */
typedef struct _PUBLICKEYBLOBHDR
{
	BLOBHEADER      header;
	RSAPUBKEY       rsapubkey;
} PUBLICKEYBLOBHDR;
/* Followed by modulus, rsapubkey.bitlen bits long, little endian */

/* Note that this is the same as PUBLICKEYBLOBHDR, just what follows differs */
typedef struct _PRIVATEKEYBLOBHDR
{
	BLOBHEADER      header;
	RSAPUBKEY       rsapubkey;
} PRIVATEKEYBLOBHDR;
/* Followed by (all in little endian form, bitlen rounded up to mul of 64):
 *	modulus, rsapubkey.bitlen bits long,
 *	prime1, rsapubkey.bitlen/2 bits long,
 *  prime2, rsapubkey.bitlen/2 bits long, (prime1 > prime2)
 *  exponent1, rsapubkey.bitlen/2 bits long (privateExponent % (prime1-1))
 *  exponent2, rsapubkey.bitlen/2 bits long (privateExponent % (prime2-1))
 *	coefficient, rsapubkey.bitlen/2 bits long (inverse of prime2 mod prime1)
 *  privateExponent, rsapubkey.bitlen bits long
 */


typedef struct _SIMPLEBLOBHDR
{
	BLOBHEADER      header;
	ALG_ID          algid;		/* of pubkey which encrypts session key */
} SIMPLEBLOBHDR;
/* Followed by session key, encrypted in PKCS-1 format with a public key */


/* Cache loadability information */
static PGPBoolean sLoadabilityTested = FALSE;
static PGPUInt32 sCAPIuse = 0;
/* Cache library handles and entry point vectors */
static HINSTANCE hLibAdv;
static HINSTANCE hLibCry;
#if !PGP_USECAPIFORMD2
static
#endif
       CAPIEntries pgp_CAPI_externs;	/* May be used by MD2 CAPI code */

#define DUMMYBITS 384

/*
 * Dummy key, 384 bits, with public exponent = private exponent = 1
 * Used when we don't want CAPI to do anything with our data.
 */
static PGPByte dummyKeyBlob[] = {
	/* Blob header */
	PRIVATEKEYBLOB, BVERSION, 0, 0, B4(CALG_RSA_KEYX),
	/* RSAPUBKEY values */
	B4(RSAPRIVMAGIC), B4(DUMMYBITS), B4(1),

	/* Modulus */
	0xc9, 0xd7, 0x0e, 0x21, 0xbb, 0xe0, 0xb5, 0x7a,
	0x09, 0x3f, 0x64, 0x2d, 0xd7, 0xd0, 0x5c, 0xd6,
	0x3b, 0xb3, 0xed, 0x00, 0xe3, 0x2f, 0x53, 0xd9,
	0x26, 0xee, 0x47, 0x2f, 0x3e, 0x40, 0x5f, 0xc0,
	0xf3, 0xb4, 0x25, 0x55, 0xea, 0xcf, 0x0b, 0x8f,
	0x08, 0x28, 0x1a, 0xdd, 0x3b, 0x2a, 0x49, 0x8c,
	/* Prime1 */
	0xe7, 0xc0, 0xd7, 0x29, 0x28, 0x71, 0xe3, 0xc4,
	0x05, 0x8c, 0xee, 0x1b, 0x37, 0xf4, 0x81, 0xe6,
	0x5e, 0xb8, 0x4d, 0x38, 0xd3, 0x56, 0x80, 0xfa,
	/* Prime2 */
	0xcf, 0x9b, 0x02, 0x98, 0x55, 0x23, 0x09, 0xc6,
	0x44, 0x82, 0x69, 0x63, 0xae, 0x68, 0xe1, 0xd2,
	0x25, 0xfc, 0xe6, 0xe6, 0xc4, 0x7b, 0x5d, 0x8f,
	/* Exponent1 */
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* Exponent2 */
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* Coefficient */
	0x12, 0x12, 0xcd, 0x7f, 0x49, 0x92, 0xdf, 0x92,
	0x3e, 0x0c, 0xc6, 0x76, 0x8e, 0x6e, 0x72, 0xd7,
	0xe6, 0x94, 0xdf, 0xd8, 0xc5, 0x7c, 0x7f, 0x1e,
	/* Private exponent */
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};




/* Returns TRUE if we were able to find everything and can use CAPI */
static PGPBoolean
LoadCAPIEntryPoints ()
{
	PGPBoolean success = FALSE;

	if (!(ex.CryptAcquireContext = GetProcAddress (hLibAdv,
				"CryptAcquireContextA")) &&
		!(ex.CryptAcquireContext = GetProcAddress (hLibCry,
				"CryptAcquireContextA")))
		goto done;
	if (!(ex.CryptReleaseContext = GetProcAddress (hLibAdv,
				"CryptReleaseContext")) &&
		!(ex.CryptReleaseContext = GetProcAddress (hLibCry,
				"CryptReleaseContext")))
		goto done;
	if (!(ex.CryptImportKey = GetProcAddress (hLibAdv,
				"CryptImportKey")) &&
		!(ex.CryptImportKey = GetProcAddress (hLibCry,
				"CryptImportKey")))
		goto done;
	if (!(ex.CryptExportKey = GetProcAddress (hLibAdv,
				"CryptExportKey")) &&
		!(ex.CryptExportKey = GetProcAddress (hLibCry,
				"CryptExportKey")))
		goto done;
	if (!(ex.CryptDestroyKey = GetProcAddress (hLibAdv,
				"CryptDestroyKey")) &&
		!(ex.CryptDestroyKey = GetProcAddress (hLibCry,
				"CryptDestroyKey")))
		goto done;
	if (!(ex.CryptCreateHash = GetProcAddress (hLibAdv,
				"CryptCreateHash")) &&
		!(ex.CryptCreateHash = GetProcAddress (hLibCry,
				"CryptCreateHash")))
		goto done;
	if (!(ex.CryptHashData = GetProcAddress (hLibAdv,
				"CryptHashData")) &&
		!(ex.CryptHashData = GetProcAddress (hLibCry,
				"CryptHashData")))
		goto done;
	if (!(ex.CryptSetHashParam = GetProcAddress (hLibAdv,
				"CryptSetHashParam")) &&
		!(ex.CryptSetHashParam = GetProcAddress (hLibCry,
				"CryptSetHashParam")))
		goto done;
	if (!(ex.CryptDestroyHash = GetProcAddress (hLibAdv,
				"CryptDestroyHash")) &&
		!(ex.CryptDestroyHash = GetProcAddress (hLibCry,
				"CryptDestroyHash")))
		goto done;
	if (!(ex.CryptVerifySignature = GetProcAddress (hLibAdv,
				"CryptVerifySignatureA")) &&
		!(ex.CryptVerifySignature = GetProcAddress (hLibCry,
				"CryptVerifySignatureA")))
		goto done;
	if (!(ex.CryptSignHash = GetProcAddress (hLibAdv,
				"CryptSignHashA")) &&
		!(ex.CryptSignHash = GetProcAddress (hLibCry,
				"CryptSignHashA")))
		goto done;
	success = TRUE;

done:
	if (!success) {
		if (hLibAdv)
			FreeLibrary (hLibAdv);
		if (hLibCry)
			FreeLibrary (hLibCry);
	}
	return success;
}


/* Test what CAPI features are available */
/* The compiler seems to be storing above the stack frame */
/* Try turning off optimizer */
#pragma optimize ("", off)
PGPUInt32
pgpCAPIuse ()
{
	HCRYPTPROV hProv;
	PGPBoolean DEFLoadable, ENHLoadable;

	if (!sLoadabilityTested) {
		sLoadabilityTested = TRUE;
		sCAPIuse = 0;
		hLibAdv = LoadLibrary ("advapi32.dll");
		if (!hLibAdv)
			return 0;
		hLibCry = LoadLibrary ("crypt32.dll");
		if (!LoadCAPIEntryPoints ())
			return 0;
		/* See if DEF provider is available */
		DEFLoadable = ex.CryptAcquireContext (&hProv, NULL, MS_DEF_PROV,
							PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
		if (DEFLoadable)
			ex.CryptReleaseContext (hProv);
		ENHLoadable = ex.CryptAcquireContext (&hProv, NULL, MS_ENH_PROV,
							PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
		if (ENHLoadable)
			ex.CryptReleaseContext (hProv);
		sCAPIuse = (DEFLoadable ? PGP_PKUSE_SIGN : 0) |
				   (ENHLoadable ? PGP_PKUSE_ENCRYPT : 0);
	}
	return sCAPIuse;
}
/* Restore default optimizations */
#pragma optimize ("", on)


/*
 * This returns nonzero if the key is too big, returning the
 * maximum number of bits that the library can accept.
 * We will also use it to check for constraints on how CAPI works.
 * This may mean that we will return an error even when the key is
 * smaller than the max.
 */
#define MAXSIZE 4096
int
rsaKeyTooBig(RSApub const *pub, RSAsec const *sec)
{
	if (pub) {
		if (bnBits(&pub->n) > MAXSIZE)
			return MAXSIZE;

		if (bnBytes(&pub->e) > sizeof(DWORD))
			return MAXSIZE;
	}
	if (sec) {
		if (bnBits(&sec->n) > MAXSIZE)
			return MAXSIZE;

		if (bnBytes(&sec->e) > sizeof(DWORD))
			return MAXSIZE;
	}
	return 0; /* OK */
}

/* Tell the size needed for a PUBLICKEYBLOB from a RSApub */
static PGPSize
rpubk_size(RSApub const *pub)
{
	PGPUInt32 bits = MULOF(bnBits(&pub->n), 64);
	PGPUInt32 len = bits/8;

	return sizeof(PUBLICKEYBLOBHDR) + len;
}

/* Tell the size needed for a PRIVATEKEYBLOB from a RSAsec */
static PGPSize
rprivk_size(RSAsec const *sec)
{
	PGPUInt32 bits = MULOF(bnBits(&sec->n), 64);
	PGPUInt32 len = bits/8;

	return sizeof(PRIVATEKEYBLOBHDR) + (9*len)/2;
}

/* Tell the size needed for a SIMPLEKEYBLOB given encryption mod bits */
static PGPSize
rsesk_size(PGPUInt32 modbits)
{
	modbits = MULOF(modbits, 8);
	return sizeof(SIMPLEBLOBHDR) + modbits/8;
}


/* Initialize a PUBLICKEYBLOB pubkey structure from a RSApub. */
static void
rpubk_init(PUBLICKEYBLOBHDR *rpubk, RSApub const *pub)
{
	PGPUInt32 bits = MULOF(bnBits(&pub->n), 64);
	PGPUInt32 len = bits/8;
	PGPByte *bnbuf;

	pgpClearMemory(rpubk, sizeof (*rpubk) + len);

	rpubk->header.bType = PUBLICKEYBLOB;
	rpubk->header.bVersion = BVERSION;
	rpubk->header.aiKeyAlg = CALG_RSA_KEYX;

	rpubk->rsapubkey.magic = RSAPUBMAGIC;
	rpubk->rsapubkey.bitlen = MULOF(bnBits(&pub->n), 8);
	bnExtractLittleBytes(&pub->e, (PGPByte *)&rpubk->rsapubkey.pubexp, 0,
						 sizeof(rpubk->rsapubkey.pubexp));

	bnbuf = (PGPByte *)rpubk + sizeof(*rpubk);
	bnExtractLittleBytes(&pub->n, bnbuf, 0, len);
}


/* Initialize RSAREF privkey structure from a RSAsec. */
static void
rprivk_init(PRIVATEKEYBLOBHDR *rprivk, RSAsec const *sec,
	PGPMemoryMgrRef	mgr, PGPBoolean fSign)
{
	PGPUInt32 bits = MULOF(bnBits(&sec->n), 64);
	PGPUInt32 len = bits/8;
	BigNum dmodp, dmodq, tmp;
	PGPByte *bnbuf;

	pgpClearMemory(rprivk, sizeof (*rprivk) + (9*len)/2);

	/* Calculate d mod p-1 and d mod q-1 */
	bnBegin(&dmodp, mgr, TRUE);
	bnBegin(&dmodq, mgr, TRUE);
	bnBegin(&tmp, mgr, TRUE);
	bnCopy(&tmp, &sec->p);
	bnSubQ(&tmp, 1);
	bnMod(&dmodp, &sec->d, &tmp);
	bnCopy(&tmp, &sec->q);
	bnSubQ(&tmp, 1);
	bnMod(&dmodq, &sec->d, &tmp);

	/* Fill in structure */
	rprivk->header.bType = PRIVATEKEYBLOB;
	rprivk->header.bVersion = BVERSION;
	rprivk->header.aiKeyAlg = fSign ? CALG_RSA_SIGN : CALG_RSA_KEYX;

	rprivk->rsapubkey.magic = RSAPRIVMAGIC;
	rprivk->rsapubkey.bitlen = MULOF(bnBits(&sec->n), 8);
	bnExtractLittleBytes(&sec->e, (PGPByte *)&rprivk->rsapubkey.pubexp, 0,
						 sizeof(rprivk->rsapubkey.pubexp));

	bnbuf = (PGPByte *)rprivk + sizeof(*rprivk);
	bnExtractLittleBytes(&sec->n, bnbuf, 0, len);
	bnbuf += len;
	/* CAPI uses convention p > q, the opposite of ours */
	bnExtractLittleBytes(&sec->q, bnbuf, 0, len/2);
	bnbuf += len/2;
	bnExtractLittleBytes(&sec->p, bnbuf, 0, len/2);
	bnbuf += len/2;
	bnExtractLittleBytes(&dmodq, bnbuf, 0, len/2);
	bnbuf += len/2;
	bnExtractLittleBytes(&dmodp, bnbuf, 0, len/2);
	bnbuf += len/2;
	bnExtractLittleBytes(&sec->u, bnbuf, 0, len/2);
	bnbuf += len/2;
	bnExtractLittleBytes(&sec->d, bnbuf, 0, len);

	bnEnd(&dmodp);
	bnEnd(&dmodq);
	bnEnd(&tmp);
}


/*
 * Initialize a SIMPLEBLOB structure from a buffer, where the blob
 * will be decrypted with a public key of the specified size in bits
 * Note that rsesk is sensitive and should be allocated securely.
 */
static void
rsesk_initfrom_buf(SIMPLEBLOBHDR *rsesk, PGPByte const *in, unsigned len,
				   PGPUInt32 modbits, PGPMemoryMgrRef mgr,
				   PGPRandomContext const *rc)
{
	PGPUInt32 modlen;
	PGPByte *bnbuf;
	BigNum skbn;

	modbits = MULOF(modbits, 8);
	modlen = modbits / 8;

	pgpClearMemory(rsesk, sizeof (*rsesk) + modlen);

	rsesk->header.bType = SIMPLEBLOB;
	rsesk->header.bVersion = BVERSION;
	rsesk->header.aiKeyAlg = CALG_3DES;
	rsesk->algid = CALG_RSA_KEYX;

	bnbuf = (PGPByte *)rsesk + sizeof(*rsesk);

	bnBegin (&skbn, mgr, TRUE);
	pgpPKCSPack (&skbn, in, len, PKCS_PAD_ENCRYPTED, modlen, rc);
	bnExtractLittleBytes (&skbn, bnbuf, 0, modlen);

	bnEnd (&skbn);
}

/*
 * Initialize a SIMPLEBLOB structure from a buffer, where the blob
 * will be decrypted with a public key of the specified size in bits
 */
static void
rsesk_initfrom_bn(SIMPLEBLOBHDR *rsesk, BigNum *bn, PGPUInt32 modbits)
{
	PGPUInt32 modlen;
	PGPByte *bnbuf;

	modbits = MULOF(modbits, 8);
	modlen = modbits / 8;

	pgpClearMemory(rsesk, sizeof (*rsesk) + modlen);

	rsesk->header.bType = SIMPLEBLOB;
	rsesk->header.bVersion = BVERSION;
	rsesk->header.aiKeyAlg = CALG_3DES;
	rsesk->algid = CALG_RSA_KEYX;

	bnbuf = (PGPByte *)rsesk + sizeof(*rsesk);
	bnExtractLittleBytes (bn, bnbuf, 0, modlen);
}


/*
 * Encrypt a buffer holding a session key with an RSA public key
 */
int
rsaPublicEncrypt(BigNum *bn, PGPByte const *in, unsigned len,
	RSApub const *pub, PGPRandomContext const *rc)
{
	HCRYPTPROV hProv = 0;
	HCRYPTKEY hDummy = 0;
	HCRYPTKEY hKey = 0;
	HCRYPTKEY hSession = 0;
	PUBLICKEYBLOBHDR *rpubk = NULL;
	SIMPLEBLOBHDR *rsesk = NULL;
	PGPUInt32 rpubksize;
	PGPUInt32 rsesksize;
	PGPByte *rseskdata;
	PGPMemoryMgrRef mgr = pub->n.mgr;
	PGPError err = kPGPError_NoErr;

	/* Set the return number to 0 to start */
	(void)bnSetQ(bn, 0);

	if (!(pgpCAPIuse() & PGP_PKUSE_ENCRYPT))
		return kPGPError_PublicKeyUnimplemented;

	/* Get handle to the enhanced PROV_RSA_FULL provider */
	/* Use our reserved container name, always start with an empty one */
	ex.CryptAcquireContext(&hProv, PGPCONTAINER, MS_ENH_PROV, PROV_RSA_FULL,
						CRYPT_DELETEKEYSET);
	if(!ex.CryptAcquireContext(&hProv, PGPCONTAINER, MS_ENH_PROV,
						PROV_RSA_FULL, CRYPT_NEWKEYSET)) {
		err = kPGPError_PublicKeyUnimplemented;
		goto error;
	}

	/* Initialize CAPI public key structure */
	rpubksize = rpubk_size (pub);

	rpubk = PGPNewData (mgr, rpubksize, 0);
	if (IsNull (rpubk)) {
		err = kPGPError_OutOfMemory;
		goto error;
	}

	rpubk_init(rpubk, pub);

	/* Create dummy key for importing session key */
	if (!ex.CryptImportKey (hProv, (BYTE *)&dummyKeyBlob,
						sizeof(dummyKeyBlob), 0, 0, &hDummy)) {
		err = kPGPError_PublicKeyUnimplemented;
		goto error;
	}

	/* Initialize and import session key */
	rsesksize = rsesk_size (DUMMYBITS);

	rsesk = PGPNewSecureData (mgr, rsesksize, 0);
	if (IsNull (rsesk)) {
		err = kPGPError_OutOfMemory;
		goto error;
	}
							
	rsesk_initfrom_buf (rsesk, in, len, DUMMYBITS, mgr, rc);

	if (!ex.CryptImportKey (hProv, (BYTE *)rsesk, rsesksize, hDummy,
						 CRYPT_EXPORTABLE, &hSession)) {
		err = kPGPError_PublicKeyUnimplemented;
		goto error;
	}
	PGPFreeData (rsesk);
	rsesk = NULL;

	/* Export session key now encrypted with user's public key */
	if (!ex.CryptImportKey (hProv, (BYTE *)rpubk, rpubksize, 0, 0, &hKey)) {
		err = kPGPError_PublicKeyUnimplemented;
		goto error;
	}

	/* CAPI ovewrites more data than it should with nonstandard key sizes */
	rsesksize = sizeof(*rsesk) + MULOF(bnBytes(&pub->n), 8);
	rsesk = (SIMPLEBLOBHDR *)PGPNewData (mgr, rsesksize, 0);
	if (!rsesk) {
		err = kPGPError_OutOfMemory;
		goto error;
	}

	pgpAssert (sizeof(rsesksize) == sizeof(DWORD));
	if (!ex.CryptExportKey (hSession, hKey, SIMPLEBLOB, 0, (BYTE *)rsesk,
						   &rsesksize)) {
		err = kPGPError_PublicKeyUnimplemented;
		goto error;
	}
	pgpAssert (rsesksize == sizeof(*rsesk) + bnBytes(&pub->n));

	/* Return in PGP bn format */
	rseskdata = (PGPByte *)rsesk + sizeof(*rsesk);
	bnInsertLittleBytes(bn, rseskdata, 0, bnBytes(&pub->n));

error:

	if (hSession)
		ex.CryptDestroyKey (hSession);
	if (hKey)
		ex.CryptDestroyKey (hKey);
	if (hDummy)
		ex.CryptDestroyKey (hDummy);

	ex.CryptReleaseContext(hProv, 0);
	ex.CryptAcquireContext(&hProv, PGPCONTAINER, MS_ENH_PROV, PROV_RSA_FULL,
							CRYPT_DELETEKEYSET);

	if (rsesk) {
		pgpClearMemory (rsesk, rsesksize);
		PGPFreeData (rsesk);
	}
	if (rpubk) {
		pgpClearMemory (rpubk, rpubksize);
		PGPFreeData (rpubk);
	}

	return err;
}



/*
 * Performs an RSA decryption.  Returns a prefix of the unwrapped
 * data in the given buf.  Returns the length of the untruncated
 * data, which may exceed "len". Returns <0 on error.
 */
int
rsaPrivateDecrypt(PGPByte *outbuf, unsigned len, BigNum *bn,
	RSAsec const *sec)
{
	HCRYPTPROV hProv = 0;
	HCRYPTKEY hDummy = 0;
	HCRYPTKEY hKey = 0;
	HCRYPTKEY hSession = 0;
	PRIVATEKEYBLOBHDR *rprivk = NULL;
	SIMPLEBLOBHDR *rsesk = NULL;
	PGPUInt32 rprivksize;
	PGPUInt32 rsesksize;
	PGPByte *rseskdata;
	PGPMemoryMgrRef	mgr	= bn->mgr;
	PGPError err = kPGPError_NoErr;
	BigNum bnsk;

	if (!(pgpCAPIuse() & PGP_PKUSE_ENCRYPT))
		return kPGPError_PublicKeyUnimplemented;

	/* CAPI has bugs with keys not mult of 64 bits */
	if ((bnBytes(bn) % 8) != 0)
		return kPGPError_CAPIUnsupportedKey;
		
	/* Get handle to the enhanced PROV_RSA_FULL provider */
	/* Use our reserved container name, always start with an empty one */
	ex.CryptAcquireContext(&hProv, PGPCONTAINER, MS_ENH_PROV, PROV_RSA_FULL,
						CRYPT_DELETEKEYSET);
	if(!ex.CryptAcquireContext(&hProv, PGPCONTAINER, MS_ENH_PROV,
						PROV_RSA_FULL, CRYPT_NEWKEYSET)) {
		err = kPGPError_PublicKeyUnimplemented;
		goto error;
	}

	/* Initialize and import private key structure */
	rprivksize = rprivk_size (sec);
	rprivk = PGPNewSecureData (mgr, rprivksize, 0);
	if (IsNull (rprivk)) {
		err = kPGPError_OutOfMemory;
		goto error;
	}
	rprivk_init(rprivk, sec, mgr, FALSE);

	if (!ex.CryptImportKey (hProv, (BYTE *)rprivk, rprivksize, 0, 0, &hKey)) {
		err = kPGPError_PublicKeyUnimplemented;
		goto error;
	}

	/* Create SIMPLEBLOB from encrypted session key in bn */
	rsesksize = rsesk_size (bnBits(&sec->n));

	rsesk = PGPNewData (mgr, rsesksize, 0);
	if (IsNull (rsesk)) {
		err = kPGPError_OutOfMemory;
		goto error;
	}
	rsesk_initfrom_bn (rsesk, bn, bnBits(&sec->n));

	/* Import the blob, decrypting it with the private key */
	if (!ex.CryptImportKey (hProv, (BYTE *)rsesk, rsesksize, hKey,
						 CRYPT_EXPORTABLE, &hSession)) {
		err = kPGPError_PublicKeyUnimplemented;
		goto error;
	}
	PGPFreeData (rsesk);
	rsesk = NULL;

	/* Create a dummy key suitable for clearly exporting the session key */
	if (!ex.CryptImportKey (hProv, (BYTE *)&dummyKeyBlob,
						   sizeof(dummyKeyBlob), 0, 0, &hDummy)) {
		err = kPGPError_PublicKeyUnimplemented;
		goto error;
	}

	/* Export session key in the clear as a PKCS1 formatted SIMPLEBLOB */
	rsesksize = sizeof (*rsesk) + DUMMYBITS/8;
	rsesk = PGPNewData (mgr, rsesksize, 0);
	if (IsNull (rsesk)) {
		err = kPGPError_OutOfMemory;
		goto error;
	}

	pgpAssert (sizeof(rsesksize) == sizeof(DWORD));
	if (!ex.CryptExportKey (hSession, hDummy, SIMPLEBLOB, 0, (BYTE *)rsesk,
						   &rsesksize)) {
		err = kPGPError_PublicKeyUnimplemented;
		goto error;
	}
	pgpAssert (rsesksize == sizeof(*rsesk) + DUMMYBITS/8);

	/* Unpack PKCS formatted exported session key and return it */
	rseskdata = (PGPByte *)rsesk + sizeof(*rsesk);
	bnBegin (&bnsk, mgr, TRUE);
	bnInsertLittleBytes (&bnsk, rseskdata, 0, DUMMYBITS/8);
	err = (PGPError) pgpPKCSUnpack(outbuf, len, &bnsk, PKCS_PAD_ENCRYPTED,
								   DUMMYBITS/8);
	bnEnd (&bnsk);

error:

	if (hSession)
		ex.CryptDestroyKey (hSession);
	if (hKey)
		ex.CryptDestroyKey (hKey);
	if (hDummy)
		ex.CryptDestroyKey (hDummy);

	ex.CryptReleaseContext(hProv, 0);
	ex.CryptAcquireContext(&hProv, PGPCONTAINER, MS_ENH_PROV, PROV_RSA_FULL,
							CRYPT_DELETEKEYSET);

	if (rsesk) {
		pgpClearMemory (rsesk, rsesksize);
		PGPFreeData (rsesk);
	}
	if (rprivk) {
		pgpClearMemory (rprivk, rprivksize);
		PGPFreeData (rprivk);
	}

	return err;
}



/*
 * Do an RSA signing operation using the secret key on the specified hash
 */
int
rsaSignHash(BigNum *bn, RSAsec const *sec, PGPHashVTBL const *h,
	PGPByte const *hash)
{
	HCRYPTPROV hProv = 0;
	HCRYPTKEY hKey = 0;
	HCRYPTHASH hHash = 0;
	PRIVATEKEYBLOBHDR *rprivk = NULL;
	ALG_ID capihashalg;
	PGPUInt32 rprivksize;
	PGPByte *sigbuf = NULL;
	PGPUInt32 sigsize;
	PGPMemoryMgrRef	mgr	= bn->mgr;
	PGPError err = kPGPError_NoErr;

	/* SET the return number to 0 to start */
	(void)bnSetQ(bn, 0);

	if (!(pgpCAPIuse() & PGP_PKUSE_SIGN))
		return kPGPError_PublicKeyUnimplemented;

	/* CAPI has bugs with keys not mult of 64 bits */
	if ((bnBytes(&sec->n) % 8) != 0)
		return kPGPError_CAPIUnsupportedKey;
		
	/* Get handle to the base PROV_RSA_FULL provider */
	/* Use our reserved container name, always start with an empty one */
	ex.CryptAcquireContext(&hProv, PGPCONTAINER, MS_DEF_PROV, PROV_RSA_FULL,
						CRYPT_DELETEKEYSET);
	if(!ex.CryptAcquireContext(&hProv, PGPCONTAINER, MS_DEF_PROV,
							PROV_RSA_FULL, CRYPT_NEWKEYSET)) {
		err = kPGPError_PublicKeyUnimplemented;
		goto error;
	}

	/* Determine what hash algorithm to use */
	if (h->algorithm == kPGPHashAlgorithm_MD5) {
		capihashalg = CALG_MD5;
	} else if (h->algorithm == kPGPHashAlgorithm_MD2) {
		capihashalg = CALG_MD2;
	} else if (h->algorithm == kPGPHashAlgorithm_SHA) {
		capihashalg = CALG_SHA;
	} else {
		err = kPGPError_PublicKeyUnimplemented;
		goto error;
	}

	/* Create and load the hash value */
	if (!ex.CryptCreateHash (hProv, capihashalg, 0, 0, &hHash)) {
		err = kPGPError_PublicKeyUnimplemented;
		goto error;
	}
	if (!ex.CryptSetHashParam (hHash, HP_HASHVAL, (BYTE *)hash, 0)) {
		err = kPGPError_PublicKeyUnimplemented;
		goto error;
	}

	/* Initialize private key structure */
	rprivksize = rprivk_size (sec);
	rprivk = PGPNewSecureData (mgr, rprivksize, 0);
	if (IsNull (rprivk)) {
		err = kPGPError_OutOfMemory;
		goto error;
	}
	rprivk_init(rprivk, sec, mgr, TRUE);

	/*
	 * Importing private key into an otherwise empty key container should
	 * make it be used as the signing key.
	 */

	if (!ex.CryptImportKey (hProv, (BYTE *)rprivk, rprivksize, 0, 0, &hKey)) {
		err = kPGPError_PublicKeyUnimplemented;
		goto error;
	}

	/* Perform the signature operation */
	sigsize = bnBytes (&sec->n);
	sigbuf = PGPNewData (mgr, sigsize, 0);
	if (IsNull (sigbuf)) {
		err = kPGPError_OutOfMemory;
		goto error;
	}

	pgpAssert (sizeof(sigsize) == sizeof(DWORD));
	if (!ex.CryptSignHash (hHash, AT_SIGNATURE, 0, 0, sigbuf, &sigsize)) {
		err = kPGPError_PublicKeyUnimplemented;
		sigsize = 0;
		goto error;
	}
	pgpAssert (sigsize = bnBytes(&sec->n));

	/* Return in PGP bn format */
	bnInsertLittleBytes(bn, sigbuf, 0, sigsize);
	
error:

	if (hKey)
		ex.CryptDestroyKey (hKey);
	if (hHash)
		ex.CryptDestroyHash (hHash);

	ex.CryptReleaseContext(hProv, 0);
	ex.CryptAcquireContext(&hProv, PGPCONTAINER, MS_ENH_PROV, PROV_RSA_FULL,
							CRYPT_DELETEKEYSET);

	if (sigbuf) {
		pgpClearMemory (sigbuf, sigsize);
		PGPFreeData (sigbuf);
	}
	if (rprivk) {
		pgpClearMemory (rprivk, rprivksize);
		PGPFreeData (rprivk);
	}

	return err;
}


/*
 * Return TRUE if the bignum holding the signature value is valid.
 */
PGPBoolean
rsaVerifyHashSignature(BigNum *bn, RSApub const *pub, PGPHashVTBL const *h,
	PGPByte const *hash)
{
	HCRYPTPROV hProv = 0;
	HCRYPTKEY hKey = 0;
	HCRYPTHASH hHash = 0;
	PUBLICKEYBLOBHDR *rpubk = NULL;
	ALG_ID capihashalg;
	PGPUInt32 rpubksize;
	PGPByte *sigbuf = NULL;
	PGPUInt32 sigsize;
	PGPMemoryMgrRef	mgr	= bn->mgr;
	PGPBoolean valid = FALSE;

	if (!(pgpCAPIuse() & PGP_PKUSE_SIGN))
		return FALSE;

	/* Get handle to the base PROV_RSA_FULL provider */
	/* Signature verification does not need a key container */

#if PGP_USECAPIFORMD2	/* [ */
	if (h->algorithm == kPGPHashAlgorithm_MD2) {
		hProv = ((HCRYPTPROV *)hash)[0];
	} else
#endif
	{
		if (!ex.CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL,
									CRYPT_VERIFYCONTEXT)) {
			goto error;
		}
	}

	/* Determine what hash algorithm to use */
	if (h->algorithm == kPGPHashAlgorithm_MD5) {
		capihashalg = CALG_MD5;
	} else if (h->algorithm == kPGPHashAlgorithm_MD2) {
		capihashalg = CALG_MD2;
	} else if (h->algorithm == kPGPHashAlgorithm_SHA) {
		capihashalg = CALG_SHA;
	} else {
		goto error;
	}

	/* Create and load the hash value */
#if PGP_USECAPIFORMD2	/* [ */
	if (h->algorithm == kPGPHashAlgorithm_MD2) {
		hHash = ((HCRYPTHASH *)hash)[1];
	} else
#endif /* ] */
	{
		if (!ex.CryptCreateHash (hProv, capihashalg, 0, 0, &hHash)) {
			goto error;
		}
		if (!ex.CryptSetHashParam (hHash, HP_HASHVAL, (BYTE *)hash, 0)) {
			goto error;
		}
	}

	/* Initialize and import public key structure */
	rpubksize = rpubk_size (pub);
	rpubk = PGPNewData (mgr, rpubksize, 0);
	if (IsNull (rpubk)) {
		goto error;
	}
	rpubk_init(rpubk, pub);

	if (!ex.CryptImportKey (hProv, (BYTE *)rpubk, rpubksize, 0, 0, &hKey)) {
		goto error;
	}

	/* Extract and verify signature */
	sigsize = bnBytes(&pub->n);
	sigbuf = PGPNewData (mgr, sigsize, 0);
	if (IsNull (sigbuf)) {
		goto error;
	}

	bnExtractLittleBytes(bn, sigbuf, 0, sigsize);	

	valid = ex.CryptVerifySignature (hHash, sigbuf, sigsize, hKey, 0, 0);

error:

	if (hKey)
		ex.CryptDestroyKey (hKey);
	if (hHash)
		ex.CryptDestroyHash (hHash);

	ex.CryptReleaseContext(hProv, 0);

	if (sigbuf) {
		pgpClearMemory (sigbuf, sigsize);
		PGPFreeData (sigbuf);
	}
	if (rpubk) {
		pgpClearMemory (rpubk, rpubksize);
		PGPFreeData (rpubk);
	}

	return valid;
}


#endif	/* ] PGP_USECAPIFORRSA */
