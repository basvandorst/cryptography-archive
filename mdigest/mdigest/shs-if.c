
/* -------------------------------------------------------------------- */
/*                                                                      */
/*  Secure Asyncronous Communications Subsystem (SACS)                  */
/*  (C) 1994 Matthew Gream <M.Gream@uts.edu.au>                         */
/*                                                                      */
/*  You have been given a limited license to use and distribute this    */
/*  library. You should have received this license with the original    */
/*  release package of this library, named "license.txt" and entitled   */
/*  "SACS SOFTWARE LICENSE". In the event that you have not read or     */
/*  understood this license, regardless of reason, you may do nothing   */
/*  with this library other than read it for self-awareness.            */
/*                                                                      */
/* -------------------------------------------------------------------- */

/* $Id: shs-if.c%v 0.90 1994/02/22 20:37:24 matt Exp $
 */

/*
 *  File:         shs-if.c
 *  Description:  interface to nist shs
 */

#include <mdigest.h>
#include "shs.h"


/* -------------------------------------------------------------------- */
/* function    : SHS_FINAL                                              */
/* description : perform the final operation, so as to copy in digest   */
/* -------------------------------------------------------------------- */
STATIC void
SHS_FINAL (shsInfo, digest)
    SHS_INFO*       shsInfo;        /* private shsinfo structure        */
    opaque          digest;         /* where to place digest            */
{
    shsFinal (shsInfo);
    memcpy (digest, shsInfo->digest, SHS_DIGESTSIZE);
}

/* -------------------------------------------------------------------- */
/* function    : SHS_CREATE                                             */
/* description : individually setup a SHS context                       */
/* -------------------------------------------------------------------- */
STATIC err_code
SHS_CREATE (ctx)
    md_ctx*         ctx;            /* context to setup                 */
{
    if (ctx->priv == NULL)
      ((SHS_INFO*)ctx->priv) = (SHS_INFO*)mem_get (sizeof(SHS_INFO), MEMOPT_CLEAR);

    return (err_ok);
}


/* -------------------------------------------------------------------- */
/* function    : SHS_DESTROY                                            */
/* description : shutdown a previously allocated context                */
/* -------------------------------------------------------------------- */
STATIC err_code
SHS_DESTROY (ctx)
    md_ctx*         ctx;            /* context to deallocate            */
{
    if (ctx->priv != NULL)
      mem_free (ctx->priv);

    ctx->priv = NULL;
    return (err_ok);
}


/* -------------------------------------------------------------------- */
/* function    : SHS_REGISTER                                           */
/* description : register and setup SHS info structure                  */
/* -------------------------------------------------------------------- */
GLOBAL err_code
SHS_REGISTER (m)
    md_info*        m;              /* info structure to fill out       */
{
    m->block_sz     = SHS_BLOCKSIZE;
    m->hash_sz      = SHS_DIGESTSIZE;

    m->create       = SHS_CREATE;
    m->destroy      = SHS_DESTROY;
    m->deregister   = NULL;

    m->init         = shsInit;
    m->update       = shsUpdate;
    m->final        = SHS_FINAL;

    return (err_ok);
}

/* -------------------------------------------------------------------- */

