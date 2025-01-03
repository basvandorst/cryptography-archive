/* lib/ssl/ssl_err.c */
/* Copyright (C) 1995-1997 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
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
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
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
#include "err.h"
#include "ssl.h"

/* BEGIN ERROR CODES */
static ERR_STRING_DATA SSL_str_functs[]=
	{
{ERR_PACK(0,SSL_F_CLIENT_CERTIFICATE,0),	"CLIENT_CERTIFICATE"},
{ERR_PACK(0,SSL_F_CLIENT_HELLO,0),	"CLIENT_HELLO"},
{ERR_PACK(0,SSL_F_CLIENT_MASTER_KEY,0),	"CLIENT_MASTER_KEY"},
{ERR_PACK(0,SSL_F_D2I_SSL_SESSION,0),	"d2i_SSL_SESSION"},
{ERR_PACK(0,SSL_F_DO_SSL3_WRITE,0),	"DO_SSL3_WRITE"},
{ERR_PACK(0,SSL_F_GET_CLIENT_FINISHED,0),	"GET_CLIENT_FINISHED"},
{ERR_PACK(0,SSL_F_GET_CLIENT_HELLO,0),	"GET_CLIENT_HELLO"},
{ERR_PACK(0,SSL_F_GET_CLIENT_MASTER_KEY,0),	"GET_CLIENT_MASTER_KEY"},
{ERR_PACK(0,SSL_F_GET_SERVER_FINISHED,0),	"GET_SERVER_FINISHED"},
{ERR_PACK(0,SSL_F_GET_SERVER_HELLO,0),	"GET_SERVER_HELLO"},
{ERR_PACK(0,SSL_F_GET_SERVER_VERIFY,0),	"GET_SERVER_VERIFY"},
{ERR_PACK(0,SSL_F_I2D_SSL_SESSION,0),	"i2d_SSL_SESSION"},
{ERR_PACK(0,SSL_F_READ_N,0),	"READ_N"},
{ERR_PACK(0,SSL_F_REQUEST_CERTIFICATE,0),	"REQUEST_CERTIFICATE"},
{ERR_PACK(0,SSL_F_SERVER_HELLO,0),	"SERVER_HELLO"},
{ERR_PACK(0,SSL_F_SSL23_ACCEPT,0),	"SSL23_ACCEPT"},
{ERR_PACK(0,SSL_F_SSL23_CLIENT_HELLO,0),	"SSL23_CLIENT_HELLO"},
{ERR_PACK(0,SSL_F_SSL23_CONNECT,0),	"SSL23_CONNECT"},
{ERR_PACK(0,SSL_F_SSL23_GET_CLIENT_HELLO,0),	"SSL23_GET_CLIENT_HELLO"},
{ERR_PACK(0,SSL_F_SSL23_GET_SERVER_HELLO,0),	"SSL23_GET_SERVER_HELLO"},
{ERR_PACK(0,SSL_F_SSL23_READ,0),	"SSL23_READ"},
{ERR_PACK(0,SSL_F_SSL23_WRITE,0),	"SSL23_WRITE"},
{ERR_PACK(0,SSL_F_SSL2_ACCEPT,0),	"SSL2_ACCEPT"},
{ERR_PACK(0,SSL_F_SSL2_CONNECT,0),	"SSL2_CONNECT"},
{ERR_PACK(0,SSL_F_SSL2_ENC_INIT,0),	"SSL2_ENC_INIT"},
{ERR_PACK(0,SSL_F_SSL2_READ,0),	"SSL2_READ"},
{ERR_PACK(0,SSL_F_SSL2_SET_CERTIFICATE,0),	"SSL2_SET_CERTIFICATE"},
{ERR_PACK(0,SSL_F_SSL2_WRITE,0),	"SSL2_WRITE"},
{ERR_PACK(0,SSL_F_SSL3_ACCEPT,0),	"SSL3_ACCEPT"},
{ERR_PACK(0,SSL_F_SSL3_CHANGE_CIPHER_STATE,0),	"SSL3_CHANGE_CIPHER_STATE"},
{ERR_PACK(0,SSL_F_SSL3_CHECK_CERT_AND_ALGORITHM,0),	"SSL3_CHECK_CERT_AND_ALGORITHM"},
{ERR_PACK(0,SSL_F_SSL3_CLIENT_HELLO,0),	"SSL3_CLIENT_HELLO"},
{ERR_PACK(0,SSL_F_SSL3_CONNECT,0),	"SSL3_CONNECT"},
{ERR_PACK(0,SSL_F_SSL3_CTX_CTRL,0),	"SSL3_CTX_CTRL"},
{ERR_PACK(0,SSL_F_SSL3_ENC,0),	"SSL3_ENC"},
{ERR_PACK(0,SSL_F_SSL3_GET_CERTIFICATE_REQUEST,0),	"SSL3_GET_CERTIFICATE_REQUEST"},
{ERR_PACK(0,SSL_F_SSL3_GET_CERT_VERIFY,0),	"SSL3_GET_CERT_VERIFY"},
{ERR_PACK(0,SSL_F_SSL3_GET_CLIENT_CERTIFICATE,0),	"SSL3_GET_CLIENT_CERTIFICATE"},
{ERR_PACK(0,SSL_F_SSL3_GET_CLIENT_HELLO,0),	"SSL3_GET_CLIENT_HELLO"},
{ERR_PACK(0,SSL_F_SSL3_GET_CLIENT_KEY_EXCHANGE,0),	"SSL3_GET_CLIENT_KEY_EXCHANGE"},
{ERR_PACK(0,SSL_F_SSL3_GET_FINISHED,0),	"SSL3_GET_FINISHED"},
{ERR_PACK(0,SSL_F_SSL3_GET_KEY_EXCHANGE,0),	"SSL3_GET_KEY_EXCHANGE"},
{ERR_PACK(0,SSL_F_SSL3_GET_MESSAGE,0),	"SSL3_GET_MESSAGE"},
{ERR_PACK(0,SSL_F_SSL3_GET_RECORD,0),	"SSL3_GET_RECORD"},
{ERR_PACK(0,SSL_F_SSL3_GET_SERVER_CERTIFICATE,0),	"SSL3_GET_SERVER_CERTIFICATE"},
{ERR_PACK(0,SSL_F_SSL3_GET_SERVER_DONE,0),	"SSL3_GET_SERVER_DONE"},
{ERR_PACK(0,SSL_F_SSL3_GET_SERVER_HELLO,0),	"SSL3_GET_SERVER_HELLO"},
{ERR_PACK(0,SSL_F_SSL3_OUTPUT_CERT_CHAIN,0),	"SSL3_OUTPUT_CERT_CHAIN"},
{ERR_PACK(0,SSL_F_SSL3_READ_BYTES,0),	"SSL3_READ_BYTES"},
{ERR_PACK(0,SSL_F_SSL3_READ_N,0),	"SSL3_READ_N"},
{ERR_PACK(0,SSL_F_SSL3_SEND_CERTIFICATE_REQUEST,0),	"SSL3_SEND_CERTIFICATE_REQUEST"},
{ERR_PACK(0,SSL_F_SSL3_SEND_CLIENT_CERTIFICATE,0),	"SSL3_SEND_CLIENT_CERTIFICATE"},
{ERR_PACK(0,SSL_F_SSL3_SEND_CLIENT_KEY_EXCHANGE,0),	"SSL3_SEND_CLIENT_KEY_EXCHANGE"},
{ERR_PACK(0,SSL_F_SSL3_SEND_CLIENT_VERIFY,0),	"SSL3_SEND_CLIENT_VERIFY"},
{ERR_PACK(0,SSL_F_SSL3_SEND_SERVER_CERTIFICATE,0),	"SSL3_SEND_SERVER_CERTIFICATE"},
{ERR_PACK(0,SSL_F_SSL3_SEND_SERVER_KEY_EXCHANGE,0),	"SSL3_SEND_SERVER_KEY_EXCHANGE"},
{ERR_PACK(0,SSL_F_SSL3_SETUP_BUFFERS,0),	"SSL3_SETUP_BUFFERS"},
{ERR_PACK(0,SSL_F_SSL3_SETUP_KEY_BLOCK,0),	"SSL3_SETUP_KEY_BLOCK"},
{ERR_PACK(0,SSL_F_SSL3_WRITE_BYTES,0),	"SSL3_WRITE_BYTES"},
{ERR_PACK(0,SSL_F_SSL3_WRITE_PENDING,0),	"SSL3_WRITE_PENDING"},
{ERR_PACK(0,SSL_F_SSL_BAD_METHOD,0),	"SSL_BAD_METHOD"},
{ERR_PACK(0,SSL_F_SSL_BYTES_TO_CIPHER_LIST,0),	"SSL_BYTES_TO_CIPHER_LIST"},
{ERR_PACK(0,SSL_F_SSL_CERT_NEW,0),	"SSL_CERT_NEW"},
{ERR_PACK(0,SSL_F_SSL_CHECK_PRIVATE_KEY,0),	"SSL_check_private_key"},
{ERR_PACK(0,SSL_F_SSL_CREATE_CIPHER_LIST,0),	"SSL_CREATE_CIPHER_LIST"},
{ERR_PACK(0,SSL_F_SSL_CTX_CHECK_PRIVATE_KEY,0),	"SSL_CTX_check_private_key"},
{ERR_PACK(0,SSL_F_SSL_CTX_NEW,0),	"SSL_CTX_new"},
{ERR_PACK(0,SSL_F_SSL_CTX_SET_SSL_VERSION,0),	"SSL_CTX_set_ssl_version"},
{ERR_PACK(0,SSL_F_SSL_CTX_USE_CERTIFICATE,0),	"SSL_CTX_use_certificate"},
{ERR_PACK(0,SSL_F_SSL_CTX_USE_CERTIFICATE_ASN1,0),	"SSL_CTX_use_certificate_ASN1"},
{ERR_PACK(0,SSL_F_SSL_CTX_USE_CERTIFICATE_FILE,0),	"SSL_CTX_use_certificate_file"},
{ERR_PACK(0,SSL_F_SSL_CTX_USE_PRIVATEKEY,0),	"SSL_CTX_use_PrivateKey"},
{ERR_PACK(0,SSL_F_SSL_CTX_USE_PRIVATEKEY_ASN1,0),	"SSL_CTX_use_PrivateKey_ASN1"},
{ERR_PACK(0,SSL_F_SSL_CTX_USE_PRIVATEKEY_FILE,0),	"SSL_CTX_use_PrivateKey_file"},
{ERR_PACK(0,SSL_F_SSL_CTX_USE_RSAPRIVATEKEY,0),	"SSL_CTX_use_RSAPrivateKey"},
{ERR_PACK(0,SSL_F_SSL_CTX_USE_RSAPRIVATEKEY_ASN1,0),	"SSL_CTX_use_RSAPrivateKey_ASN1"},
{ERR_PACK(0,SSL_F_SSL_CTX_USE_RSAPRIVATEKEY_FILE,0),	"SSL_CTX_use_RSAPrivateKey_file"},
{ERR_PACK(0,SSL_F_SSL_DO_HANDSHAKE,0),	"SSL_do_handshake"},
{ERR_PACK(0,SSL_F_SSL_GET_NEW_SESSION,0),	"SSL_GET_NEW_SESSION"},
{ERR_PACK(0,SSL_F_SSL_GET_SERVER_SEND_CERT,0),	"SSL_GET_SERVER_SEND_CERT"},
{ERR_PACK(0,SSL_F_SSL_GET_SIGN_PKEY,0),	"SSL_GET_SIGN_PKEY"},
{ERR_PACK(0,SSL_F_SSL_LOAD_CLIENT_CA_FILE,0),	"SSL_load_client_CA_file"},
{ERR_PACK(0,SSL_F_SSL_NEW,0),	"SSL_new"},
{ERR_PACK(0,SSL_F_SSL_RSA_PRIVATE_DECRYPT,0),	"SSL_RSA_PRIVATE_DECRYPT"},
{ERR_PACK(0,SSL_F_SSL_RSA_PUBLIC_ENCRYPT,0),	"SSL_RSA_PUBLIC_ENCRYPT"},
{ERR_PACK(0,SSL_F_SSL_SESSION_NEW,0),	"SSL_SESSION_new"},
{ERR_PACK(0,SSL_F_SSL_SESSION_PRINT_FP,0),	"SSL_SESSION_print_fp"},
{ERR_PACK(0,SSL_F_SSL_SET_FD,0),	"SSL_set_fd"},
{ERR_PACK(0,SSL_F_SSL_SET_PKEY,0),	"SSL_SET_PKEY"},
{ERR_PACK(0,SSL_F_SSL_SET_RFD,0),	"SSL_set_rfd"},
{ERR_PACK(0,SSL_F_SSL_SET_SESSION,0),	"SSL_set_session"},
{ERR_PACK(0,SSL_F_SSL_SET_WFD,0),	"SSL_set_wfd"},
{ERR_PACK(0,SSL_F_SSL_UNDEFINED_FUNCTION,0),	"SSL_UNDEFINED_FUNCTION"},
{ERR_PACK(0,SSL_F_SSL_USE_CERTIFICATE,0),	"SSL_use_certificate"},
{ERR_PACK(0,SSL_F_SSL_USE_CERTIFICATE_ASN1,0),	"SSL_use_certificate_ASN1"},
{ERR_PACK(0,SSL_F_SSL_USE_CERTIFICATE_FILE,0),	"SSL_use_certificate_file"},
{ERR_PACK(0,SSL_F_SSL_USE_PRIVATEKEY,0),	"SSL_use_PrivateKey"},
{ERR_PACK(0,SSL_F_SSL_USE_PRIVATEKEY_ASN1,0),	"SSL_use_PrivateKey_ASN1"},
{ERR_PACK(0,SSL_F_SSL_USE_PRIVATEKEY_FILE,0),	"SSL_use_PrivateKey_file"},
{ERR_PACK(0,SSL_F_SSL_USE_RSAPRIVATEKEY,0),	"SSL_use_RSAPrivateKey"},
{ERR_PACK(0,SSL_F_SSL_USE_RSAPRIVATEKEY_ASN1,0),	"SSL_use_RSAPrivateKey_ASN1"},
{ERR_PACK(0,SSL_F_SSL_USE_RSAPRIVATEKEY_FILE,0),	"SSL_use_RSAPrivateKey_file"},
{ERR_PACK(0,SSL_F_SSL_WRITE,0),	"SSL_write"},
{ERR_PACK(0,SSL_F_WRITE_PENDING,0),	"WRITE_PENDING"},
{0,NULL},
	};

static ERR_STRING_DATA SSL_str_reasons[]=
	{
{SSL_R_APP_DATA_IN_HANDSHAKE             ,"app data in handshake"},
{SSL_R_BAD_ALERT_RECORD                  ,"bad alert record"},
{SSL_R_BAD_AUTHENTICATION_TYPE           ,"bad authentication type"},
{SSL_R_BAD_CHANGE_CIPHER_SPEC            ,"bad change cipher spec"},
{SSL_R_BAD_CHECKSUM                      ,"bad checksum"},
{SSL_R_BAD_CLIENT_REQUEST                ,"bad client request"},
{SSL_R_BAD_DATA_RETURNED_BY_CALLBACK     ,"bad data returned by callback"},
{SSL_R_BAD_DECOMPRESSION                 ,"bad decompression"},
{SSL_R_BAD_DH_G_LENGTH                   ,"bad dh g length"},
{SSL_R_BAD_DH_PUB_KEY_LENGTH             ,"bad dh pub key length"},
{SSL_R_BAD_DH_P_LENGTH                   ,"bad dh p length"},
{SSL_R_BAD_DIGEST_LENGTH                 ,"bad digest length"},
{SSL_R_BAD_DSA_SIGNATURE                 ,"bad dsa signature"},
{SSL_R_BAD_MAC_DECODE                    ,"bad mac decode"},
{SSL_R_BAD_MESSAGE_TYPE                  ,"bad message type"},
{SSL_R_BAD_PACKET_LENGTH                 ,"bad packet length"},
{SSL_R_BAD_RESPONSE_ARGUMENT             ,"bad response argument"},
{SSL_R_BAD_RSA_DECRYPT                   ,"bad rsa decrypt"},
{SSL_R_BAD_RSA_ENCRYPT                   ,"bad rsa encrypt"},
{SSL_R_BAD_RSA_E_LENGTH                  ,"bad rsa e length"},
{SSL_R_BAD_RSA_MODULUS_LENGTH            ,"bad rsa modulus length"},
{SSL_R_BAD_RSA_SIGNATURE                 ,"bad rsa signature"},
{SSL_R_BAD_SIGNATURE                     ,"bad signature"},
{SSL_R_BAD_SSL_FILETYPE                  ,"bad ssl filetype"},
{SSL_R_BAD_SSL_SESSION_ID_LENGTH         ,"bad ssl session id length"},
{SSL_R_BAD_STATE                         ,"bad state"},
{SSL_R_BAD_WRITE_RETRY                   ,"bad write retry"},
{SSL_R_BIO_NOT_SET                       ,"bio not set"},
{SSL_R_BLOCK_CIPHER_PAD_IS_WRONG         ,"block cipher pad is wrong"},
{SSL_R_BN_LIB                            ,"bn lib"},
{SSL_R_CA_DN_LENGTH_MISMATCH             ,"ca dn length mismatch"},
{SSL_R_CA_DN_TOO_LONG                    ,"ca dn too long"},
{SSL_R_CCS_RECEIVED_EARLY                ,"ccs received early"},
{SSL_R_CERTIFICATE_VERIFY_FAILED         ,"certificate verify failed"},
{SSL_R_CERT_LENGTH_MISMATCH              ,"cert length mismatch"},
{SSL_R_CHALLENGE_IS_DIFFERENT            ,"challenge is different"},
{SSL_R_CIPHER_CODE_WRONG_LENGTH          ,"cipher code wrong length"},
{SSL_R_CIPHER_OR_HASH_UNAVAILABLE        ,"cipher or hash unavailable"},
{SSL_R_CIPHER_TABLE_SRC_ERROR            ,"cipher table src error"},
{SSL_R_COMPRESSED_LENGTH_TOO_LONG        ,"compressed length too long"},
{SSL_R_COMPRESSION_FAILURE               ,"compression failure"},
{SSL_R_CONNECTION_ID_IS_DIFFERENT        ,"connection id is different"},
{SSL_R_DATA_BETWEEN_CCS_AND_FINISHED     ,"data between ccs and finished"},
{SSL_R_DATA_LENGTH_TOO_LONG              ,"data length too long"},
{SSL_R_DH_PUBLIC_VALUE_LENGTH_IS_WRONG   ,"dh public value length is wrong"},
{SSL_R_DIGEST_CHECK_FAILED               ,"digest check failed"},
{SSL_R_ENCRYPTED_LENGTH_TOO_LONG         ,"encrypted length too long"},
{SSL_R_ERROR_IN_RECEIVED_CIPHER_LIST     ,"error in received cipher list"},
{SSL_R_EXCESSIVE_MESSAGE_SIZE            ,"excessive message size"},
{SSL_R_EXTRA_DATA_IN_MESSAGE             ,"extra data in message"},
{SSL_R_GOT_A_FIN_BEFORE_A_CCS            ,"got a fin before a ccs"},
{SSL_R_INTERNAL_ERROR                    ,"internal error"},
{SSL_R_INVALID_CHALLENGE_LENGTH          ,"invalid challenge length"},
{SSL_R_LENGTH_MISMATCH                   ,"length mismatch"},
{SSL_R_LENGTH_TOO_SHORT                  ,"length too short"},
{SSL_R_LIBRARY_HAS_NO_CIPHERS            ,"library has no ciphers"},
{SSL_R_MISSING_DH_DSA_CERT               ,"missing dh dsa cert"},
{SSL_R_MISSING_DH_KEY                    ,"missing dh key"},
{SSL_R_MISSING_DH_RSA_CERT               ,"missing dh rsa cert"},
{SSL_R_MISSING_DSA_SIGNING_CERT          ,"missing dsa signing cert"},
{SSL_R_MISSING_EXPORT_TMP_DH_KEY         ,"missing export tmp dh key"},
{SSL_R_MISSING_EXPORT_TMP_RSA_KEY        ,"missing export tmp rsa key"},
{SSL_R_MISSING_RSA_CERTIFICATE           ,"missing rsa certificate"},
{SSL_R_MISSING_RSA_ENCRYPTING_CERT       ,"missing rsa encrypting cert"},
{SSL_R_MISSING_RSA_SIGNING_CERT          ,"missing rsa signing cert"},
{SSL_R_MISSING_TMP_DH_KEY                ,"missing tmp dh key"},
{SSL_R_MISSING_TMP_RSA_KEY               ,"missing tmp rsa key"},
{SSL_R_MISSING_TMP_RSA_PKEY              ,"missing tmp rsa pkey"},
{SSL_R_MISSING_VERIFY_MESSAGE            ,"missing verify message"},
{SSL_R_NON_SSLV2_INITIAL_PACKET          ,"non sslv2 initial packet"},
{SSL_R_NO_CERTIFICATES_PASSED            ,"no certificates passed"},
{SSL_R_NO_CERTIFICATE_ASSIGNED           ,"no certificate assigned"},
{SSL_R_NO_CERTIFICATE_RETURNED           ,"no certificate returned"},
{SSL_R_NO_CERTIFICATE_SET                ,"no certificate set"},
{SSL_R_NO_CERTIFICATE_SPECIFIED          ,"no certificate specified"},
{SSL_R_NO_CIPHERS_AVAILABLE              ,"no ciphers available"},
{SSL_R_NO_CIPHERS_PASSED                 ,"no ciphers passed"},
{SSL_R_NO_CIPHERS_SPECIFIED              ,"no ciphers specified"},
{SSL_R_NO_CIPHER_LIST                    ,"no cipher list"},
{SSL_R_NO_CIPHER_MATCH                   ,"no cipher match"},
{SSL_R_NO_CLIENT_CERT_RECEIVED           ,"no client cert received"},
{SSL_R_NO_COMPRESSION_SPECIFIED          ,"no compression specified"},
{SSL_R_NO_PRIVATEKEY                     ,"no privatekey"},
{SSL_R_NO_PRIVATE_KEY_ASSIGNED           ,"no private key assigned"},
{SSL_R_NO_PUBLICKEY                      ,"no publickey"},
{SSL_R_NO_SHARED_CIPHER                  ,"no shared cipher"},
{SSL_R_NULL_SSL_CTX                      ,"null ssl ctx"},
{SSL_R_NULL_SSL_METHOD_PASSED            ,"null ssl method passed"},
{SSL_R_OLD_SESSION_CIPHER_NOT_RETURNED   ,"old session cipher not returned"},
{SSL_R_PACKET_LENGTH_TOO_LONG            ,"packet length too long"},
{SSL_R_PEER_DID_NOT_RETURN_A_CERTIFICATE ,"peer did not return a certificate"},
{SSL_R_PEER_ERROR                        ,"peer error"},
{SSL_R_PEER_ERROR_CERTIFICATE            ,"peer error certificate"},
{SSL_R_PEER_ERROR_NO_CERTIFICATE         ,"peer error no certificate"},
{SSL_R_PEER_ERROR_NO_CIPHER              ,"peer error no cipher"},
{SSL_R_PEER_ERROR_UNSUPPORTED_CERTIFICATE_TYPE,"peer error unsupported certificate type"},
{SSL_R_PRE_MAC_LENGTH_TOO_LONG           ,"pre mac length too long"},
{SSL_R_PROBLEMS_MAPPING_CIPHER_FUNCTIONS ,"problems mapping cipher functions"},
{SSL_R_PROTOCOL_IS_SHUTDOWN              ,"protocol is shutdown"},
{SSL_R_PUBLIC_KEY_ENCRYPT_ERROR          ,"public key encrypt error"},
{SSL_R_PUBLIC_KEY_IS_NOT_RSA             ,"public key is not rsa"},
{SSL_R_PUBLIC_KEY_NOT_RSA                ,"public key not rsa"},
{SSL_R_READ_BIO_NOT_SET                  ,"read bio not set"},
{SSL_R_READ_WRONG_PACKET_TYPE            ,"read wrong packet type"},
{SSL_R_RECORD_LENGTH_MISMATCH            ,"record length mismatch"},
{SSL_R_RECORD_TOO_LARGE                  ,"record too large"},
{SSL_R_REQUIRED_CIPHER_MISSING           ,"required cipher missing"},
{SSL_R_REUSE_CERT_LENGTH_NOT_ZERO        ,"reuse cert length not zero"},
{SSL_R_REUSE_CERT_TYPE_NOT_ZERO          ,"reuse cert type not zero"},
{SSL_R_REUSE_CIPHER_LIST_NOT_ZERO        ,"reuse cipher list not zero"},
{SSL_R_SHORT_READ                        ,"short read"},
{SSL_R_SIGNATURE_FOR_NON_SIGNING_CERTIFICATE,"signature for non signing certificate"},
{SSL_R_SSL3_SESSION_ID_TOO_SHORT         ,"ssl3 session id too short"},
{SSL_R_SSLV3_ALERT_BAD_CERTIFICATE       ,"sslv3 alert bad certificate"},
{SSL_R_SSLV3_ALERT_BAD_RECORD_MAC        ,"sslv3 alert bad record mac"},
{SSL_R_SSLV3_ALERT_CERTIFICATE_EXPIRED   ,"sslv3 alert certificate expired"},
{SSL_R_SSLV3_ALERT_CERTIFICATE_REVOKED   ,"sslv3 alert certificate revoked"},
{SSL_R_SSLV3_ALERT_CERTIFICATE_UNKNOWN   ,"sslv3 alert certificate unknown"},
{SSL_R_SSLV3_ALERT_DECOMPRESSION_FAILURE ,"sslv3 alert decompression failure"},
{SSL_R_SSLV3_ALERT_HANDSHAKE_FAILURE     ,"sslv3 alert handshake failure"},
{SSL_R_SSLV3_ALERT_ILLEGAL_PARAMETER     ,"sslv3 alert illegal parameter"},
{SSL_R_SSLV3_ALERT_NO_CERTIFICATE        ,"sslv3 alert no certificate"},
{SSL_R_SSLV3_ALERT_PEER_ERROR_CERTIFICATE,"sslv3 alert peer error certificate"},
{SSL_R_SSLV3_ALERT_PEER_ERROR_NO_CERTIFICATE,"sslv3 alert peer error no certificate"},
{SSL_R_SSLV3_ALERT_PEER_ERROR_NO_CIPHER  ,"sslv3 alert peer error no cipher"},
{SSL_R_SSLV3_ALERT_PEER_ERROR_UNSUPPORTED_CERTIFICATE_TYPE,"sslv3 alert peer error unsupported certificate type"},
{SSL_R_SSLV3_ALERT_UNEXPECTED_MESSAGE    ,"sslv3 alert unexpected message"},
{SSL_R_SSLV3_ALERT_UNKNOWN_REMOTE_ERROR_TYPE,"sslv3 alert unknown remote error type"},
{SSL_R_SSLV3_ALERT_UNSUPPORTED_CERTIFICATE,"sslv3 alert unsupported certificate"},
{SSL_R_SSL_CTX_HAS_NO_DEFAULT_SSL_VERSION,"ssl ctx has no default ssl version"},
{SSL_R_SSL_HANDSHAKE_FAILURE             ,"ssl handshake failure"},
{SSL_R_SSL_LIBRARY_HAS_NO_CIPHERS        ,"ssl library has no ciphers"},
{SSL_R_SSL_SESSION_ID_IS_DIFFERENT       ,"ssl session id is different"},
{SSL_R_TRIED_TO_USE_UNSUPPORTED_CIPHER   ,"tried to use unsupported cipher"},
{SSL_R_UNABLE_TO_DECODE_DH_CERTS         ,"unable to decode dh certs"},
{SSL_R_UNABLE_TO_EXTRACT_PUBLIC_KEY      ,"unable to extract public key"},
{SSL_R_UNABLE_TO_FIND_DH_PARAMETERS      ,"unable to find dh parameters"},
{SSL_R_UNABLE_TO_FIND_PUBLIC_KEY_PARAMETERS,"unable to find public key parameters"},
{SSL_R_UNABLE_TO_FIND_SSL_METHOD         ,"unable to find ssl method"},
{SSL_R_UNEXPECTED_MESSAGE                ,"unexpected message"},
{SSL_R_UNEXPECTED_RECORD                 ,"unexpected record"},
{SSL_R_UNKNOWN_ALERT_TYPE                ,"unknown alert type"},
{SSL_R_UNKNOWN_CERTIFICATE_TYPE          ,"unknown certificate type"},
{SSL_R_UNKNOWN_CIPHER_RETURNED           ,"unknown cipher returned"},
{SSL_R_UNKNOWN_CIPHER_TYPE               ,"unknown cipher type"},
{SSL_R_UNKNOWN_KEY_EXCHANGE_TYPE         ,"unknown key exchange type"},
{SSL_R_UNKNOWN_PKEY_TYPE                 ,"unknown pkey type"},
{SSL_R_UNKNOWN_PROTOCOL                  ,"unknown protocol"},
{SSL_R_UNKNOWN_REMOTE_ERROR_TYPE         ,"unknown remote error type"},
{SSL_R_UNKNOWN_SSL_VERSION               ,"unknown ssl version"},
{SSL_R_UNKNOWN_STATE                     ,"unknown state"},
{SSL_R_UNSUPPORTED_CIPHER                ,"unsupported cipher"},
{SSL_R_UNSUPPORTED_COMPRESSION_ALGORITHM ,"unsupported compression algorithm"},
{SSL_R_UNSUPPORTED_SSL_VERSION           ,"unsupported ssl version"},
{SSL_R_WRITE_BIO_NOT_SET                 ,"write bio not set"},
{SSL_R_WRONG_CIPHER_RETURNED             ,"wrong cipher returned"},
{SSL_R_WRONG_MESSAGE_TYPE                ,"wrong message type"},
{SSL_R_WRONG_NUMBER_OF_KEY_BITS          ,"wrong number of key bits"},
{SSL_R_WRONG_SIGNATURE_LENGTH            ,"wrong signature length"},
{SSL_R_WRONG_SIGNATURE_SIZE              ,"wrong signature size"},
{SSL_R_WRONG_SSL_VERSION                 ,"wrong ssl version"},
{SSL_R_WRONG_VERSION_NUMBER              ,"wrong version number"},
{SSL_R_X509_LIB                          ,"x509 lib"},
{0,NULL},
	};

void ERR_load_SSL_strings()
	{
	static int init=1;

	if (init)
		{
		init=0;
		ERR_load_strings(ERR_LIB_SSL,SSL_str_functs);
		ERR_load_strings(ERR_LIB_SSL,SSL_str_reasons);
		}
	}
