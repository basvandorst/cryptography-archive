/* lib/asn1/asn1_err.c */
/* Copyright (C) 1995 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 * 
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SS
 * specification.  This library and applications are
 * FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are aheared to.
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIA
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdio.h>
#include "crypto.h"
#include "asn1.h"

/* BEGIN ERROR CODES */
static ERR_STRING_DATA ASN1_str_functs[]=
	{
{ERR_PACK(0,ASN1_F_ASN1_BIT_STRING_NEW,0),	"ASN1_BIT_STRING_NEW"},
{ERR_PACK(0,ASN1_F_ASN1_D2I_FP,0),	"ASN1_d2i_fp"},
{ERR_PACK(0,ASN1_F_ASN1_DUP,0),	"ASN1_dup"},
{ERR_PACK(0,ASN1_F_ASN1_GET_OBJECT,0),	"ASN1_get_object"},
{ERR_PACK(0,ASN1_F_ASN1_I2D_FP,0),	"ASN1_i2d_fp"},
{ERR_PACK(0,ASN1_F_ASN1_INTEGER_SET,0),	"ASN1_INTEGER_set"},
{ERR_PACK(0,ASN1_F_ASN1_OBJECT_NEW,0),	"ASN1_OBJECT_new"},
{ERR_PACK(0,ASN1_F_ASN1_SET,0),	"ASN1_SET"},
{ERR_PACK(0,ASN1_F_ASN1_SIGN,0),	"ASN1_SIGN"},
{ERR_PACK(0,ASN1_F_ASN1_TYPE_NEW,0),	"ASN1_TYPE_new"},
{ERR_PACK(0,ASN1_F_ASN1_UTCTIME_DUP,0),	"ASN1_UTCTIME_dup"},
{ERR_PACK(0,ASN1_F_ASN1_UTCTIME_NEW,0),	"ASN1_UTCTIME_new"},
{ERR_PACK(0,ASN1_F_ASN1_VERIFY,0),	"ASN1_VERIFY"},
{ERR_PACK(0,ASN1_F_BIGNUM_TO_ASN1_INTEGER,0),	"BIGNUM_to_ASN1_INTEGER"},
{ERR_PACK(0,ASN1_F_D2I_ASN1_BIT_STRING,0),	"d2i_ASN1_BIT_STRING"},
{ERR_PACK(0,ASN1_F_D2I_ASN1_BYTES,0),	"d2i_ASN1_bytes"},
{ERR_PACK(0,ASN1_F_D2I_ASN1_INTEGER,0),	"d2i_ASN1_INTEGER"},
{ERR_PACK(0,ASN1_F_D2I_ASN1_OBJECT,0),	"d2i_ASN1_OBJECT"},
{ERR_PACK(0,ASN1_F_D2I_ASN1_OCTET_STRING,0),	"d2i_ASN1_OCTET_STRING"},
{ERR_PACK(0,ASN1_F_D2I_ASN1_TYPE,0),	"d2i_ASN1_TYPE"},
{ERR_PACK(0,ASN1_F_D2I_ASN1_UTCTIME,0),	"d2i_ASN1_UTCTIME"},
{ERR_PACK(0,ASN1_F_D2I_DHPARAMS,0),	"D2I_DHPARAMS"},
{ERR_PACK(0,ASN1_F_D2I_NETSCAPE_PKEY,0),	"D2I_NETSCAPE_PKEY"},
{ERR_PACK(0,ASN1_F_D2I_NETSCAPE_RSA,0),	"D2I_NETSCAPE_RSA"},
{ERR_PACK(0,ASN1_F_D2I_NETSCAPE_RSA_2,0),	"D2I_NETSCAPE_RSA_2"},
{ERR_PACK(0,ASN1_F_D2I_PKCS7,0),	"D2I_PKCS7"},
{ERR_PACK(0,ASN1_F_D2I_PKCS7_DIGEST,0),	"D2I_PKCS7_DIGEST"},
{ERR_PACK(0,ASN1_F_D2I_PKCS7_ENCRYPT,0),	"D2I_PKCS7_ENCRYPT"},
{ERR_PACK(0,ASN1_F_D2I_PKCS7_ENC_CONTENT,0),	"D2I_PKCS7_ENC_CONTENT"},
{ERR_PACK(0,ASN1_F_D2I_PKCS7_ENVELOPE,0),	"D2I_PKCS7_ENVELOPE"},
{ERR_PACK(0,ASN1_F_D2I_PKCS7_ISSUER_AND_SERIAL,0),	"D2I_PKCS7_ISSUER_AND_SERIAL"},
{ERR_PACK(0,ASN1_F_D2I_PKCS7_RECIP_INFO,0),	"D2I_PKCS7_RECIP_INFO"},
{ERR_PACK(0,ASN1_F_D2I_PKCS7_SIGNED,0),	"D2I_PKCS7_SIGNED"},
{ERR_PACK(0,ASN1_F_D2I_PKCS7_SIGNER_INFO,0),	"D2I_PKCS7_SIGNER_INFO"},
{ERR_PACK(0,ASN1_F_D2I_PKCS7_SIGN_ENVELOPE,0),	"D2I_PKCS7_SIGN_ENVELOPE"},
{ERR_PACK(0,ASN1_F_D2I_RSAPRIVATEKEY,0),	"D2I_RSAPRIVATEKEY"},
{ERR_PACK(0,ASN1_F_D2I_RSAPUBLICKEY,0),	"D2I_RSAPUBLICKEY"},
{ERR_PACK(0,ASN1_F_D2I_X509,0),	"D2I_X509"},
{ERR_PACK(0,ASN1_F_D2I_X509_ALGOR,0),	"D2I_X509_ALGOR"},
{ERR_PACK(0,ASN1_F_D2I_X509_ATTRIBUTE,0),	"D2I_X509_ATTRIBUTE"},
{ERR_PACK(0,ASN1_F_D2I_X509_CINF,0),	"D2I_X509_CINF"},
{ERR_PACK(0,ASN1_F_D2I_X509_CRL,0),	"D2I_X509_CRL"},
{ERR_PACK(0,ASN1_F_D2I_X509_CRL_INFO,0),	"D2I_X509_CRL_INFO"},
{ERR_PACK(0,ASN1_F_D2I_X509_KEYU,0),	"D2I_X509_KEYU"},
{ERR_PACK(0,ASN1_F_D2I_X509_NAME,0),	"D2I_X509_NAME"},
{ERR_PACK(0,ASN1_F_D2I_X509_NAME_ENTRY,0),	"D2I_X509_NAME_ENTRY"},
{ERR_PACK(0,ASN1_F_D2I_X509_PUBKEY,0),	"D2I_X509_PUBKEY"},
{ERR_PACK(0,ASN1_F_D2I_X509_REQ,0),	"D2I_X509_REQ"},
{ERR_PACK(0,ASN1_F_D2I_X509_REQ_INFO,0),	"D2I_X509_REQ_INFO"},
{ERR_PACK(0,ASN1_F_D2I_X509_REVOKED,0),	"D2I_X509_REVOKED"},
{ERR_PACK(0,ASN1_F_D2I_X509_SIG,0),	"D2I_X509_SIG"},
{ERR_PACK(0,ASN1_F_D2I_X509_VAL,0),	"D2I_X509_VAL"},
{ERR_PACK(0,ASN1_F_F2I_ASN1_INTEGER,0),	"f2i_ASN1_INTEGER"},
{ERR_PACK(0,ASN1_F_I2D_DHPARAMS,0),	"I2D_DHPARAMS"},
{ERR_PACK(0,ASN1_F_I2D_NETSCAPE_RSA_2,0),	"I2D_NETSCAPE_RSA_2"},
{ERR_PACK(0,ASN1_F_I2D_PKCS7,0),	"I2D_PKCS7"},
{ERR_PACK(0,ASN1_F_I2D_RSAPRIVATEKEY,0),	"I2D_RSAPRIVATEKEY"},
{ERR_PACK(0,ASN1_F_I2D_RSAPUBLICKEY,0),	"I2D_RSAPUBLICKEY"},
{ERR_PACK(0,ASN1_F_I2D_X509_ATTRIBUTE,0),	"I2D_X509_ATTRIBUTE"},
{ERR_PACK(0,ASN1_F_NETSCAPE_PKEY_NEW,0),	"NETSCAPE_PKEY_NEW"},
{ERR_PACK(0,ASN1_F_PKCS7_DIGEST_NEW,0),	"PKCS7_DIGEST_NEW"},
{ERR_PACK(0,ASN1_F_PKCS7_ENCRYPT_NEW,0),	"PKCS7_ENCRYPT_NEW"},
{ERR_PACK(0,ASN1_F_PKCS7_ENC_CONTENT_NEW,0),	"PKCS7_ENC_CONTENT_NEW"},
{ERR_PACK(0,ASN1_F_PKCS7_ENVELOPE_NEW,0),	"PKCS7_ENVELOPE_NEW"},
{ERR_PACK(0,ASN1_F_PKCS7_ISSUER_AND_SERIAL_NEW,0),	"PKCS7_ISSUER_AND_SERIAL_NEW"},
{ERR_PACK(0,ASN1_F_PKCS7_NEW,0),	"PKCS7_NEW"},
{ERR_PACK(0,ASN1_F_PKCS7_RECIP_INFO_NEW,0),	"PKCS7_RECIP_INFO_NEW"},
{ERR_PACK(0,ASN1_F_PKCS7_SIGNED_NEW,0),	"PKCS7_SIGNED_NEW"},
{ERR_PACK(0,ASN1_F_PKCS7_SIGNER_INFO_NEW,0),	"PKCS7_SIGNER_INFO_NEW"},
{ERR_PACK(0,ASN1_F_PKCS7_SIGN_ENVELOPE_NEW,0),	"PKCS7_SIGN_ENVELOPE_NEW"},
{ERR_PACK(0,ASN1_F_X509_ALGOR_NEW,0),	"X509_ALGOR_NEW"},
{ERR_PACK(0,ASN1_F_X509_ATTRIBUTE_NEW,0),	"X509_ATTRIBUTE_NEW"},
{ERR_PACK(0,ASN1_F_X509_CINF_NEW,0),	"X509_CINF_NEW"},
{ERR_PACK(0,ASN1_F_X509_CRL_INFO_NEW,0),	"X509_CRL_INFO_NEW"},
{ERR_PACK(0,ASN1_F_X509_CRL_NEW,0),	"X509_CRL_NEW"},
{ERR_PACK(0,ASN1_F_X509_DHPARAMS_NEW,0),	"X509_DHPARAMS_NEW"},
{ERR_PACK(0,ASN1_F_X509_INFO_NEW,0),	"X509_INFO_NEW"},
{ERR_PACK(0,ASN1_F_X509_KEY_NEW,0),	"X509_KEY_NEW"},
{ERR_PACK(0,ASN1_F_X509_NAME_ENTRY_NEW,0),	"X509_NAME_ENTRY_NEW"},
{ERR_PACK(0,ASN1_F_X509_NAME_NEW,0),	"X509_NAME_NEW"},
{ERR_PACK(0,ASN1_F_X509_NEW,0),	"X509_NEW"},
{ERR_PACK(0,ASN1_F_X509_PUBKEY_NEW,0),	"X509_PUBKEY_NEW"},
{ERR_PACK(0,ASN1_F_X509_REQ_INFO_NEW,0),	"X509_REQ_INFO_NEW"},
{ERR_PACK(0,ASN1_F_X509_REQ_NEW,0),	"X509_REQ_NEW"},
{ERR_PACK(0,ASN1_F_X509_REVOKED_NEW,0),	"X509_REVOKED_NEW"},
{ERR_PACK(0,ASN1_F_X509_SIG_NEW,0),	"X509_SIG_NEW"},
{ERR_PACK(0,ASN1_F_X509_VAL_FREE,0),	"X509_VAL_FREE"},
{ERR_PACK(0,ASN1_F_X509_VAL_NEW,0),	"X509_VAL_NEW"},
{0,NULL},
	};

static ERR_STRING_DATA ASN1_str_reasons[]=
	{
{ASN1_R_BAD_CLASS                        ,"bad class"},
{ASN1_R_BAD_GET_OBJECT                   ,"bad get object"},
{ASN1_R_BAD_OBJECT_HEADER                ,"bad object header"},
{ASN1_R_BAD_PASSWD_READ                  ,"bad passwd read"},
{ASN1_R_BAD_PASSWORD_READ                ,"bad password read"},
{ASN1_R_BAD_PKCS7_CONTENT                ,"bad pkcs7 content"},
{ASN1_R_BAD_PKCS7_TYPE                   ,"bad pkcs7 type"},
{ASN1_R_BAD_TAG                          ,"bad tag"},
{ASN1_R_BAD_TYPE                         ,"bad type"},
{ASN1_R_BIT_STRING_TOO_SHORT             ,"bit string too short"},
{ASN1_R_BN_LIB                           ,"bn lib"},
{ASN1_R_DECODING_ERROR                   ,"decoding error"},
{ASN1_R_ERROR_STACK                      ,"error stack"},
{ASN1_R_EXPECTING_AN_INTEGER             ,"expecting an integer"},
{ASN1_R_EXPECTING_AN_OBJECT              ,"expecting an object"},
{ASN1_R_EXPECTING_AN_OCTET_STRING        ,"expecting an octet string"},
{ASN1_R_EXPECTING_A_BIT_STRING           ,"expecting a bit string"},
{ASN1_R_EXPECTING_A_PRINTABLE            ,"expecting a printable"},
{ASN1_R_EXPECTING_A_SEQUENCE             ,"expecting a sequence"},
{ASN1_R_EXPECTING_A_UTCTIME              ,"expecting a utctime"},
{ASN1_R_HEADER_TOO_LONG                  ,"header too long"},
{ASN1_R_LENGTH_ERROR                     ,"length error"},
{ASN1_R_LENGTH_MISMATCH                  ,"length mismatch"},
{ASN1_R_MISSING_EOS                      ,"missing eos"},
{ASN1_R_NON_HEX_CHARACTERS               ,"non hex characters"},
{ASN1_R_NOT_ENOUGH_DATA                  ,"not enough data"},
{ASN1_R_NOT_EOC_IN_INDEFINITLE_LENGTH_SEQUENCE,"not eoc in indefinitle length sequence"},
{ASN1_R_ODD_NUMBER_OF_CHARS              ,"odd number of chars"},
{ASN1_R_PARSING                          ,"parsing"},
{ASN1_R_PRIVATE_KEY_HEADER_MISSING       ,"private key header missing"},
{ASN1_R_THE_ASN1_OBJECT_IDENTIFIER_IS_NOT_KNOWN_FOR_THIS_MD,"the asn1 object identifier is not known for this md"},
{ASN1_R_TOO_LONG                         ,"too long"},
{ASN1_R_UNABLE_TO_DECODE_RSA_KEY         ,"unable to decode rsa key"},
{ASN1_R_UNABLE_TO_DECODE_RSA_PRIVATE_KEY ,"unable to decode rsa private key"},
{ASN1_R_UNKNOWN_ATTRIBUTE_TYPE           ,"unknown attribute type"},
{ASN1_R_UNKNOWN_MESSAGE_DIGEST_ALGORITHM ,"unknown message digest algorithm"},
{ASN1_R_UNKNOWN_OBJECT_TYPE              ,"unknown object type"},
{ASN1_R_UNSUPORTED_ENCRYPTION_ALGORITHM  ,"unsuported encryption algorithm"},
{ASN1_R_UTCTIME_TOO_LONG                 ,"utctime too long"},
{ASN1_R_WRONG_EXPLICIT_TAG               ,"wrong explicit tag"},
{ASN1_R_WRONG_TAG                        ,"wrong tag"},
{0,NULL},
	};

void ERR_load_ASN1_strings()
	{
	static int init=1;

	if (init)
		{
		init=0;
		ERR_load_strings(ERR_LIB_ASN1,ASN1_str_functs);
		ERR_load_strings(ERR_LIB_ASN1,ASN1_str_reasons);
		}
	}
