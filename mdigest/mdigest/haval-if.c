
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

/* $Id: haval-if.c%v 0.90 1994/02/22 20:37:24 matt Exp $
 */

/*
 *  File:         haval-if.c
 *  Description:  interface to haval message digest
 */

#include <mdigest.h>
#include "haval.h"


/* -------------------------------------------------------------------- */
/* function    : HAVAL_CREATE                                           */
/* description : individually setup a haval context                     */
/* -------------------------------------------------------------------- */
STATIC err_code
HAVAL_CREATE (ctx)
    md_ctx*         ctx;            /* context to setup                 */
{
    if (ctx->priv == NULL)
      ((haval_state*)ctx->priv) = (haval_state*)mem_get (sizeof(haval_state), MEMOPT_CLEAR);

    return (err_ok);
}


/* -------------------------------------------------------------------- */
/* function    : HAVAL_DESTROY                                          */
/* description : shutdown a previously allocated context                */
/* -------------------------------------------------------------------- */
STATIC err_code
HAVAL_DESTROY (ctx)
    md_ctx*         ctx;            /* context to deallocate            */
{
    if (ctx->priv != NULL)
      mem_free (ctx->priv);

    ctx->priv = NULL;
    return (err_ok);
}


/* -------------------------------------------------------------------- */
/* function    : HAVAL_REGISTER                                         */
/* description : register and setup haval info structure                */
/* -------------------------------------------------------------------- */
GLOBAL err_code
HAVAL_REGISTER (m)
    md_info*        m;              /* info structure to fill out       */
{
    m->block_sz     = 128;
    m->hash_sz      = FPTLEN >> 3;

    m->create       = HAVAL_CREATE;
    m->destroy      = HAVAL_DESTROY;
    m->deregister   = NULL;

    m->init         = haval_start;
    m->update       = haval_hash;
    m->final        = haval_end;

    return (err_ok);
}

/* -------------------------------------------------------------------- */

