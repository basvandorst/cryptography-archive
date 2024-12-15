
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

/* $Id: md4-if.c%v 0.90 1994/02/22 20:37:24 matt Exp $
 */

/*
 *  File:         md4-if.c
 *  Description:  interface to md4
 */

#include <mdigest.h>
#include "md4.h"


/* -------------------------------------------------------------------- */
/* function    : MD4_CREATE                                             */
/* description : individually setup a md4 context                       */
/* -------------------------------------------------------------------- */
STATIC err_code
MD4_CREATE (ctx)
    md_ctx*         ctx;            /* context to setup                 */
{
    if (ctx->priv == NULL)
      ((MD4_CTX*)ctx->priv) = (MD4_CTX*)mem_get (sizeof(MD4_CTX), MEMOPT_CLEAR);

    return (err_ok);
}


/* -------------------------------------------------------------------- */
/* function    : MD4_DESTROY                                            */
/* description : shutdown a previously allocated context                */
/* -------------------------------------------------------------------- */
STATIC err_code
MD4_DESTROY (ctx)
    md_ctx*         ctx;            /* context to deallocate            */
{
    if (ctx->priv != NULL)
      mem_free (ctx->priv);

    ctx->priv = NULL;
    return (err_ok);
}


/* -------------------------------------------------------------------- */
/* function    : MD4_REGISTER                                           */
/* description : register and setup md4 info structure                  */
/* -------------------------------------------------------------------- */
GLOBAL err_code
MD4_REGISTER (m)
    md_info*        m;              /* info structure to fill out       */
{
    m->block_sz     = 64;
    m->hash_sz      = 16;

    m->create       = MD4_CREATE;
    m->destroy      = MD4_DESTROY;
    m->deregister   = NULL;

    m->init         = MD4Init;
    m->update       = MD4Update;
    m->final        = MD4Final;

    return (err_ok);
}

/* -------------------------------------------------------------------- */

