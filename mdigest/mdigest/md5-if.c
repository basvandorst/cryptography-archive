
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

/* $Id: md5-if.c%v 0.90 1994/02/22 20:37:24 matt Exp $
 */

/*
 *  File:         md5-if.c
 *  Description:  interface to md5
 */

#include <mdigest.h>
#include "md5.h"


/* -------------------------------------------------------------------- */
/* function    : MD5_CREATE                                             */
/* description : individually setup a md5 context                       */
/* -------------------------------------------------------------------- */
STATIC err_code
MD5_CREATE (ctx)
    md_ctx*         ctx;            /* context to setup                 */
{
    if (ctx->priv == NULL)
      ((MD5_CTX*)ctx->priv) = (MD5_CTX*)mem_get (sizeof(MD5_CTX), MEMOPT_CLEAR);

    return (err_ok);
}


/* -------------------------------------------------------------------- */
/* function    : MD5_DESTROY                                            */
/* description : shutdown a previously allocated context                */
/* -------------------------------------------------------------------- */
STATIC err_code
MD5_DESTROY (ctx)
    md_ctx*         ctx;            /* context to deallocate            */
{
    if (ctx->priv != NULL)
      mem_free (ctx->priv);

    ctx->priv = NULL;
    return (err_ok);
}


/* -------------------------------------------------------------------- */
/* function    : MD5_REGISTER                                           */
/* description : register and setup md5 info structure                  */
/* -------------------------------------------------------------------- */
GLOBAL err_code
MD5_REGISTER (m)
    md_info*        m;              /* info structure to fill out       */
{
    m->block_sz     = 64;
    m->hash_sz      = 16;

    m->create       = MD5_CREATE;
    m->destroy      = MD5_DESTROY;
    m->deregister   = NULL;

    m->init         = MD5Init;
    m->update       = MD5Update;
    m->final        = MD5Final;

    return (err_ok);
}


/* -------------------------------------------------------------------- */

