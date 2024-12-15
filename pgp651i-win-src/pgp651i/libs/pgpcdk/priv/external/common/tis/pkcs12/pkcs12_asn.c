/*******************************************************************
 *
 * This file was generated by TIS/ASN1COMP Ver. 4.1, an ASN.1 compiler.
 * TIS/ASN1COMP is Copyright (c) 1998, TIS Labs at Network Associates, Inc.
 *
 * This file was AUTOMATICALLY GENERATED on Fri Apr  9 20:37:30 1999
 *
 ******************************************************************/


#include <string.h>

#include "pkcs12_asn.h"

/*******************************************************************
 *
 * Code for manipulating ASN.1 DER data and structure.
 *
 *  NewXXX: Allocate the top level of a structure.
 *
 *  FreeXXX: Release all the data of an ASN structure.
 *
 *  SizeofXXX: Return the size of the DER block that would
 *               be created from the structure.
 *
 *  PackXXX: Take an ASN structure and return the DER.
 *
 *  unPackXXX: Take the DER and return an ASN structure.
 *
 ******************************************************************/


/******************************************************************
 * Routines for AuthenticatedSafes
 ******************************************************************/

PKIAuthenticatedSafes *PKINewAuthenticatedSafes(
    PKICONTEXT *ctx)
{
    PKIAuthenticatedSafes *f;

    if (ctx == NULL)
        return NULL;

    f = (PKIAuthenticatedSafes *)PKIAlloc(ctx->memMgr, sizeof(PKIAuthenticatedSafes));
    if (f != NULL)
        (void)memset(f, 0, sizeof(PKIAuthenticatedSafes));
    return (f);

} /* PKINewAuthenticatedSafes */

void PKIFreeAuthenticatedSafes(
    PKICONTEXT *ctx,
    PKIAuthenticatedSafes *f)
{
    if (ctx == NULL)
        return;

    PKIDropInPlaceAuthenticatedSafes(ctx, f);
    if (f != NULL)
        PKIFree(ctx->memMgr, f);

} /* PKIFreeAuthenticatedSafes */

size_t PKISizeofAuthenticatedSafes(
    PKICONTEXT *ctx,
    PKIAuthenticatedSafes *asnstruct,
    int outerSizeFlag)
{
    (void)ctx; /* for future use */

    return(PKISizeofAuthenticatedSafesInternal(asnstruct, outerSizeFlag, PKIFALSE));
} /* PKISizeofAuthenticatedSafes */

size_t PKIPackAuthenticatedSafes(
    PKICONTEXT *ctx,
    unsigned char *buf,
    size_t buflen,
    PKIAuthenticatedSafes *asnstruct,
    int *erret )
{
    return(PKIPackAuthenticatedSafesInternal(ctx, buf, buflen, asnstruct, PKIID_AuthenticatedSafes, erret));
} /* PKIPackAuthenticatedSafes */

size_t PKIUnpackAuthenticatedSafes(
    PKICONTEXT *ctx,
    PKIAuthenticatedSafes **asnstruct,
    const unsigned char *buf,
    size_t buflen,
    int *erret)
{
    size_t bytesused;
    bytesused=PKIUnpackAuthenticatedSafesInternal(ctx, asnstruct, buf, buflen,
                      PKIID_AuthenticatedSafes, erret);
    if(*erret==0 && bytesused==0)
        PKIERR(PKIErrUnpackUnderrun);
    return bytesused;
}

/******************************************************************
 * Routines for CRLBag
 ******************************************************************/

PKICRLBag *PKINewCRLBag(
    PKICONTEXT *ctx)
{
    PKICRLBag *f;

    if (ctx == NULL)
        return NULL;

    f = (PKICRLBag *)PKIAlloc(ctx->memMgr, sizeof(PKICRLBag));
    if (f != NULL)
        (void)memset(f, 0, sizeof(PKICRLBag));
    return (f);

} /* PKINewCRLBag */

void PKIFreeCRLBag(
    PKICONTEXT *ctx,
    PKICRLBag *f)
{
    if (ctx == NULL)
        return;

    PKIDropInPlaceCRLBag(ctx, f);
    if (f != NULL)
        PKIFree(ctx->memMgr, f);

} /* PKIFreeCRLBag */

size_t PKISizeofCRLBag(
    PKICONTEXT *ctx,
    PKICRLBag *asnstruct,
    int outerSizeFlag)
{
    (void)ctx; /* for future use */

    return(PKISizeofCRLBagInternal(asnstruct, outerSizeFlag, PKIFALSE));
} /* PKISizeofCRLBag */

size_t PKIPackCRLBag(
    PKICONTEXT *ctx,
    unsigned char *buf,
    size_t buflen,
    PKICRLBag *asnstruct,
    int *erret )
{
    return(PKIPackCRLBagInternal(ctx, buf, buflen, asnstruct, PKIID_CRLBag, erret));
} /* PKIPackCRLBag */

size_t PKIUnpackCRLBag(
    PKICONTEXT *ctx,
    PKICRLBag **asnstruct,
    const unsigned char *buf,
    size_t buflen,
    int *erret)
{
    size_t bytesused;
    bytesused=PKIUnpackCRLBagInternal(ctx, asnstruct, buf, buflen,
                      PKIID_CRLBag, erret);
    if(*erret==0 && bytesused==0)
        PKIERR(PKIErrUnpackUnderrun);
    return bytesused;
}

/******************************************************************
 * Routines for CertBag
 ******************************************************************/

PKICertBag *PKINewCertBag(
    PKICONTEXT *ctx)
{
    PKICertBag *f;

    if (ctx == NULL)
        return NULL;

    f = (PKICertBag *)PKIAlloc(ctx->memMgr, sizeof(PKICertBag));
    if (f != NULL)
        (void)memset(f, 0, sizeof(PKICertBag));
    return (f);

} /* PKINewCertBag */

void PKIFreeCertBag(
    PKICONTEXT *ctx,
    PKICertBag *f)
{
    if (ctx == NULL)
        return;

    PKIDropInPlaceCertBag(ctx, f);
    if (f != NULL)
        PKIFree(ctx->memMgr, f);

} /* PKIFreeCertBag */

size_t PKISizeofCertBag(
    PKICONTEXT *ctx,
    PKICertBag *asnstruct,
    int outerSizeFlag)
{
    (void)ctx; /* for future use */

    return(PKISizeofCertBagInternal(asnstruct, outerSizeFlag, PKIFALSE));
} /* PKISizeofCertBag */

size_t PKIPackCertBag(
    PKICONTEXT *ctx,
    unsigned char *buf,
    size_t buflen,
    PKICertBag *asnstruct,
    int *erret )
{
    return(PKIPackCertBagInternal(ctx, buf, buflen, asnstruct, PKIID_CertBag, erret));
} /* PKIPackCertBag */

size_t PKIUnpackCertBag(
    PKICONTEXT *ctx,
    PKICertBag **asnstruct,
    const unsigned char *buf,
    size_t buflen,
    int *erret)
{
    size_t bytesused;
    bytesused=PKIUnpackCertBagInternal(ctx, asnstruct, buf, buflen,
                      PKIID_CertBag, erret);
    if(*erret==0 && bytesused==0)
        PKIERR(PKIErrUnpackUnderrun);
    return bytesused;
}

/******************************************************************
 * Routines for MacData
 ******************************************************************/

PKIMacData *PKINewMacData(
    PKICONTEXT *ctx)
{
    PKIMacData *f;

    if (ctx == NULL)
        return NULL;

    f = (PKIMacData *)PKIAlloc(ctx->memMgr, sizeof(PKIMacData));
    if (f != NULL)
        (void)memset(f, 0, sizeof(PKIMacData));
    return (f);

} /* PKINewMacData */

void PKIFreeMacData(
    PKICONTEXT *ctx,
    PKIMacData *f)
{
    if (ctx == NULL)
        return;

    PKIDropInPlaceMacData(ctx, f);
    if (f != NULL)
        PKIFree(ctx->memMgr, f);

} /* PKIFreeMacData */

size_t PKISizeofMacData(
    PKICONTEXT *ctx,
    PKIMacData *asnstruct,
    int outerSizeFlag)
{
    (void)ctx; /* for future use */

    return(PKISizeofMacDataInternal(asnstruct, outerSizeFlag, PKIFALSE));
} /* PKISizeofMacData */

size_t PKIPackMacData(
    PKICONTEXT *ctx,
    unsigned char *buf,
    size_t buflen,
    PKIMacData *asnstruct,
    int *erret )
{
    return(PKIPackMacDataInternal(ctx, buf, buflen, asnstruct, PKIID_MacData, erret));
} /* PKIPackMacData */

size_t PKIUnpackMacData(
    PKICONTEXT *ctx,
    PKIMacData **asnstruct,
    const unsigned char *buf,
    size_t buflen,
    int *erret)
{
    size_t bytesused;
    bytesused=PKIUnpackMacDataInternal(ctx, asnstruct, buf, buflen,
                      PKIID_MacData, erret);
    if(*erret==0 && bytesused==0)
        PKIERR(PKIErrUnpackUnderrun);
    return bytesused;
}

/******************************************************************
 * Routines for SafeBag
 ******************************************************************/

PKISafeBag *PKINewSafeBag(
    PKICONTEXT *ctx)
{
    PKISafeBag *f;

    if (ctx == NULL)
        return NULL;

    f = (PKISafeBag *)PKIAlloc(ctx->memMgr, sizeof(PKISafeBag));
    if (f != NULL)
        (void)memset(f, 0, sizeof(PKISafeBag));
    return (f);

} /* PKINewSafeBag */

void PKIFreeSafeBag(
    PKICONTEXT *ctx,
    PKISafeBag *f)
{
    if (ctx == NULL)
        return;

    PKIDropInPlaceSafeBag(ctx, f);
    if (f != NULL)
        PKIFree(ctx->memMgr, f);

} /* PKIFreeSafeBag */

size_t PKISizeofSafeBag(
    PKICONTEXT *ctx,
    PKISafeBag *asnstruct,
    int outerSizeFlag)
{
    (void)ctx; /* for future use */

    return(PKISizeofSafeBagInternal(asnstruct, outerSizeFlag, PKIFALSE));
} /* PKISizeofSafeBag */

size_t PKIPackSafeBag(
    PKICONTEXT *ctx,
    unsigned char *buf,
    size_t buflen,
    PKISafeBag *asnstruct,
    int *erret )
{
    return(PKIPackSafeBagInternal(ctx, buf, buflen, asnstruct, PKIID_SafeBag, erret));
} /* PKIPackSafeBag */

size_t PKIUnpackSafeBag(
    PKICONTEXT *ctx,
    PKISafeBag **asnstruct,
    const unsigned char *buf,
    size_t buflen,
    int *erret)
{
    size_t bytesused;
    bytesused=PKIUnpackSafeBagInternal(ctx, asnstruct, buf, buflen,
                      PKIID_SafeBag, erret);
    if(*erret==0 && bytesused==0)
        PKIERR(PKIErrUnpackUnderrun);
    return bytesused;
}

/******************************************************************
 * Routines for SecretBag
 ******************************************************************/

PKISecretBag *PKINewSecretBag(
    PKICONTEXT *ctx)
{
    PKISecretBag *f;

    if (ctx == NULL)
        return NULL;

    f = (PKISecretBag *)PKIAlloc(ctx->memMgr, sizeof(PKISecretBag));
    if (f != NULL)
        (void)memset(f, 0, sizeof(PKISecretBag));
    return (f);

} /* PKINewSecretBag */

void PKIFreeSecretBag(
    PKICONTEXT *ctx,
    PKISecretBag *f)
{
    if (ctx == NULL)
        return;

    PKIDropInPlaceSecretBag(ctx, f);
    if (f != NULL)
        PKIFree(ctx->memMgr, f);

} /* PKIFreeSecretBag */

size_t PKISizeofSecretBag(
    PKICONTEXT *ctx,
    PKISecretBag *asnstruct,
    int outerSizeFlag)
{
    (void)ctx; /* for future use */

    return(PKISizeofSecretBagInternal(asnstruct, outerSizeFlag, PKIFALSE));
} /* PKISizeofSecretBag */

size_t PKIPackSecretBag(
    PKICONTEXT *ctx,
    unsigned char *buf,
    size_t buflen,
    PKISecretBag *asnstruct,
    int *erret )
{
    return(PKIPackSecretBagInternal(ctx, buf, buflen, asnstruct, PKIID_SecretBag, erret));
} /* PKIPackSecretBag */

size_t PKIUnpackSecretBag(
    PKICONTEXT *ctx,
    PKISecretBag **asnstruct,
    const unsigned char *buf,
    size_t buflen,
    int *erret)
{
    size_t bytesused;
    bytesused=PKIUnpackSecretBagInternal(ctx, asnstruct, buf, buflen,
                      PKIID_SecretBag, erret);
    if(*erret==0 && bytesused==0)
        PKIERR(PKIErrUnpackUnderrun);
    return bytesused;
}

/******************************************************************
 * Routines for pkcs_12PbeParams
 ******************************************************************/

PKIpkcs_12PbeParams *PKINewpkcs_12PbeParams(
    PKICONTEXT *ctx)
{
    PKIpkcs_12PbeParams *f;

    if (ctx == NULL)
        return NULL;

    f = (PKIpkcs_12PbeParams *)PKIAlloc(ctx->memMgr, sizeof(PKIpkcs_12PbeParams));
    if (f != NULL)
        (void)memset(f, 0, sizeof(PKIpkcs_12PbeParams));
    return (f);

} /* PKINewpkcs_12PbeParams */

void PKIFreepkcs_12PbeParams(
    PKICONTEXT *ctx,
    PKIpkcs_12PbeParams *f)
{
    if (ctx == NULL)
        return;

    PKIDropInPlacepkcs_12PbeParams(ctx, f);
    if (f != NULL)
        PKIFree(ctx->memMgr, f);

} /* PKIFreepkcs_12PbeParams */

size_t PKISizeofpkcs_12PbeParams(
    PKICONTEXT *ctx,
    PKIpkcs_12PbeParams *asnstruct,
    int outerSizeFlag)
{
    (void)ctx; /* for future use */

    return(PKISizeofpkcs_12PbeParamsInternal(asnstruct, outerSizeFlag, PKIFALSE));
} /* PKISizeofpkcs_12PbeParams */

size_t PKIPackpkcs_12PbeParams(
    PKICONTEXT *ctx,
    unsigned char *buf,
    size_t buflen,
    PKIpkcs_12PbeParams *asnstruct,
    int *erret )
{
    return(PKIPackpkcs_12PbeParamsInternal(ctx, buf, buflen, asnstruct, PKIID_pkcs_12PbeParams, erret));
} /* PKIPackpkcs_12PbeParams */

size_t PKIUnpackpkcs_12PbeParams(
    PKICONTEXT *ctx,
    PKIpkcs_12PbeParams **asnstruct,
    const unsigned char *buf,
    size_t buflen,
    int *erret)
{
    size_t bytesused;
    bytesused=PKIUnpackpkcs_12PbeParamsInternal(ctx, asnstruct, buf, buflen,
                      PKIID_pkcs_12PbeParams, erret);
    if(*erret==0 && bytesused==0)
        PKIERR(PKIErrUnpackUnderrun);
    return bytesused;
}

/******************************************************************
 * Routines for PFX
 ******************************************************************/

PKIPFX *PKINewPFX(
    PKICONTEXT *ctx)
{
    PKIPFX *f;

    if (ctx == NULL)
        return NULL;

    f = (PKIPFX *)PKIAlloc(ctx->memMgr, sizeof(PKIPFX));
    if (f != NULL)
        (void)memset(f, 0, sizeof(PKIPFX));
    return (f);

} /* PKINewPFX */

void PKIFreePFX(
    PKICONTEXT *ctx,
    PKIPFX *f)
{
    if (ctx == NULL)
        return;

    PKIDropInPlacePFX(ctx, f);
    if (f != NULL)
        PKIFree(ctx->memMgr, f);

} /* PKIFreePFX */

size_t PKISizeofPFX(
    PKICONTEXT *ctx,
    PKIPFX *asnstruct,
    int outerSizeFlag)
{
    (void)ctx; /* for future use */

    return(PKISizeofPFXInternal(asnstruct, outerSizeFlag, PKIFALSE));
} /* PKISizeofPFX */

size_t PKIPackPFX(
    PKICONTEXT *ctx,
    unsigned char *buf,
    size_t buflen,
    PKIPFX *asnstruct,
    int *erret )
{
    return(PKIPackPFXInternal(ctx, buf, buflen, asnstruct, PKIID_PFX, erret));
} /* PKIPackPFX */

size_t PKIUnpackPFX(
    PKICONTEXT *ctx,
    PKIPFX **asnstruct,
    const unsigned char *buf,
    size_t buflen,
    int *erret)
{
    size_t bytesused;
    bytesused=PKIUnpackPFXInternal(ctx, asnstruct, buf, buflen,
                      PKIID_PFX, erret);
    if(*erret==0 && bytesused==0)
        PKIERR(PKIErrUnpackUnderrun);
    return bytesused;
}

/******************************************************************
 * Routines for SafeContents
 ******************************************************************/

PKISafeContents *PKINewSafeContents(
    PKICONTEXT *ctx)
{
    PKISafeContents *f;

    if (ctx == NULL)
        return NULL;

    f = (PKISafeContents *)PKIAlloc(ctx->memMgr, sizeof(PKISafeContents));
    if (f != NULL)
        (void)memset(f, 0, sizeof(PKISafeContents));
    return (f);

} /* PKINewSafeContents */

void PKIFreeSafeContents(
    PKICONTEXT *ctx,
    PKISafeContents *f)
{
    if (ctx == NULL)
        return;

    PKIDropInPlaceSafeContents(ctx, f);
    if (f != NULL)
        PKIFree(ctx->memMgr, f);

} /* PKIFreeSafeContents */

size_t PKISizeofSafeContents(
    PKICONTEXT *ctx,
    PKISafeContents *asnstruct,
    int outerSizeFlag)
{
    (void)ctx; /* for future use */

    return(PKISizeofSafeContentsInternal(asnstruct, outerSizeFlag, PKIFALSE));
} /* PKISizeofSafeContents */

size_t PKIPackSafeContents(
    PKICONTEXT *ctx,
    unsigned char *buf,
    size_t buflen,
    PKISafeContents *asnstruct,
    int *erret )
{
    return(PKIPackSafeContentsInternal(ctx, buf, buflen, asnstruct, PKIID_SafeContents, erret));
} /* PKIPackSafeContents */

size_t PKIUnpackSafeContents(
    PKICONTEXT *ctx,
    PKISafeContents **asnstruct,
    const unsigned char *buf,
    size_t buflen,
    int *erret)
{
    size_t bytesused;
    bytesused=PKIUnpackSafeContentsInternal(ctx, asnstruct, buf, buflen,
                      PKIID_SafeContents, erret);
    if(*erret==0 && bytesused==0)
        PKIERR(PKIErrUnpackUnderrun);
    return bytesused;
}