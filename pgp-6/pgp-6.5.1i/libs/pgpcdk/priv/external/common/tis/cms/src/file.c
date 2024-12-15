/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include "tc.h"
#include <stdio.h>

#if PGP_MACINTOSH
#include <unix.h>
#else
#include <sys/stat.h>
#endif


/*****
*
* return
*   TC_E_INVARGS
*   TC_E_FILE
*   TC_E_NOMEMORY
*   0 - okay
*
*****/
int tc_read_block(
	unsigned char **p, 
	size_t *plen,
	const char *fname,
	TC_CONTEXT *ctx)
{
    FILE *f;
    struct stat st;

    if (fname == (char *)0 || p == NULL ||
        plen == (size_t *)0)
	return TC_E_INVARGS;

    if ((f = fopen (fname, "rb")) == NULL)
	return TC_E_FILE;
    if ((fstat (fileno (f), &st)) == -1)
	return TC_E_FILE;

    *plen = (size_t)st.st_size;
    if ( (*p = TC_Alloc(ctx->memMgr, (size_t)st.st_size)) == NULL) {
	*plen = 0;
	return TC_E_NOMEMORY;
    }
    if ( fread(*p, 1, (size_t)st.st_size, f) != (size_t)st.st_size)
	return TC_E_FILE;
    (void)fclose (f);

    return 0;
} /* tc_read_block */

/*****
*
* return
*
*    0 - okay
*    TC_E_INVARGS
*    TC_E_FILE
*
*****/
int tc_write_block(
	const unsigned char *p, 
	const size_t plen,
	const char *fname,
	TC_CONTEXT *ctx)
{
    FILE *f;

    (void)ctx; /* for future use */

    if (fname == (char *)0 )
	return TC_E_INVARGS;

    if (plen == 0) /* nothing to write */
	return 0;

    if ((f = fopen (fname, "wb")) == NULL)
	return TC_E_FILE;
    if ( fwrite(p, 1, plen, f) != plen)
	return TC_E_FILE;
    (void)fclose(f);

    return 0;

} /* tc_write_block */

int tc_read_cert(
    TC_CERT **cert,
    const char *fname,
    TC_CONTEXT *ctx)
{
    unsigned char *buffer = (unsigned char *)0;
    size_t buflen;
    int status = 0;
    
  do {
    if ( cert == NULL || fname == (char *)0 ) {
	status = TC_E_INVARGS;
	break;
    }

    if ( (status = tc_read_block(&buffer, &buflen, fname, ctx)) != 0)
	break;

    if ( (status = tc_unpack_cert(cert, buffer, buflen, ctx)) != 0)
	break;

  } while (/*CONSTCOND*/0);

    /* clean-up */
    if (buffer != (unsigned char *)0)
	TC_Free(ctx->memMgr, buffer);

    return status;

} /* tc_read_cert */

int tc_read_crl(
    TC_CertificateList **crl,
    const char *fname,
    TC_CONTEXT *ctx)
{
    unsigned char *buffer = (unsigned char *)0;
    size_t buflen;
    int status = 0;
    
  do {
    if ( crl == NULL || fname == (char *)0 ) {
	status = TC_E_INVARGS;
	break;
    }

    if ( (status = tc_read_block(&buffer, &buflen, fname, ctx)) != 0)
	break;

    if ( (status = tc_unpack_crl(crl, buffer, buflen, ctx)) != 0)
	break;

  } while (/*CONSTCOND*/0);

    /* clean-up */
    if (buffer != (unsigned char *)0)
	TC_Free(ctx->memMgr, buffer);

    return status;

} /* tc_read_crl */

int tc_read_request(
    TC_CertificationRequest **request,
    const char *fname,
    TC_CONTEXT *ctx)
{
    unsigned char *buffer = (unsigned char *)0;
    size_t buflen;
    int status = 0;
    
  do {
    if ( request == NULL || fname == (char *)0 ) {
	status = TC_E_INVARGS;
	break;
    }

    if ( (status = tc_read_block(&buffer, &buflen, fname, ctx)) != 0)
	break;

    if ( (status = tc_unpack_request(request, buffer, buflen, ctx)) != 0)
	break;

  } while (/*CONSTCOND*/0);

    /* clean-up */
    if (buffer != (unsigned char *)0)
	TC_Free(ctx->memMgr, buffer);

    return status;

} /* tc_read_request */

int tc_write_cert(
    TC_CERT *cert,
    const char *fname,
    TC_CONTEXT *ctx)
{
    unsigned char *buffer = (unsigned char *)0;
    size_t buflen;
    int status = 0;
    
  do {
    if ( cert == (TC_CERT *)0 || fname == (char *)0 ) {
	status = TC_E_INVARGS;
	break;
    }

    if ( (status = tc_pack_cert(&buffer, &buflen, cert, ctx)) != 0)
	break;

    if ( (status = tc_write_block(buffer, buflen, fname, ctx)) != 0)
	break;

  } while (/*CONSTCOND*/0);

    /* clean-up */
    if (buffer != (unsigned char *)0)
	TC_Free(ctx->memMgr, buffer);

    return status;

} /* tc_write_cert */

int tc_write_crl(
    TC_CertificateList *crl,
    const char *fname,
    TC_CONTEXT *ctx)
{
    unsigned char *buffer = (unsigned char *)0;
    size_t buflen;
    int status = 0;
    
  do {
    if ( crl == (TC_CertificateList *)0 || fname == (char *)0 ) {
	status = TC_E_INVARGS;
	break;
    }

    if ( (status = tc_pack_crl(&buffer, &buflen, crl, ctx)) != 0)
	break;

    if ( (status = tc_write_block(buffer, buflen, fname, ctx)) != 0)
	break;

  } while (/*CONSTCOND*/0);

    /* clean-up */
    if (buffer != (unsigned char *)0)
	TC_Free(ctx->memMgr, buffer);

    return status;

} /* tc_write_crl */

int tc_write_request(
    TC_CertificationRequest *request,
    const char *fname,
    TC_CONTEXT *ctx)
{
    unsigned char *buffer = (unsigned char *)0;
    size_t buflen;
    int status = 0;
    
  do {
    if ( request == (TC_CertificationRequest *)0 || fname == (char *)0 ) {
	status = TC_E_INVARGS;
	break;
    }

    if ( (status = tc_pack_request(&buffer, &buflen, request, ctx)) != 0)
	break;

    if ( (status = tc_write_block(buffer, buflen, fname, ctx)) != 0)
	break;

  } while (/*CONSTCOND*/0);

    /* clean-up */
    if (buffer != (unsigned char *)0)
	TC_Free(ctx->memMgr, buffer);

    return status;

} /* tc_write_request */
