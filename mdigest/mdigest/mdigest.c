
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

/* $Id: mdigest.c%v 0.90 1994/02/22 20:37:24 matt Exp $
 */

/*
 *  File:         mdigest.c
 *  Description:  generic interface for message digests
 */

#include <mdigest.h>


extern err_code		MD4_REGISTER ();
extern err_code		MD5_REGISTER ();
extern err_code		SHS_REGISTER ();
extern err_code		HAVAL_REGISTER ();

static md_info*         md_list[md_max_reg];   	/* md info buckets      */


/* -------------------------------------------------------------------- */
/* function    : md_lookup_id                                           */
/* description : look for a specific id                                 */
/* -------------------------------------------------------------------- */
GLOBAL md_info*
md_lookup_id (ident)
    string          ident;          /* the string identifier            */
{
    word            c;

    DEBUG_FUNCTION_ENTER("md_lookup_id");

    /* ---------------------------------------------------------------- */
    /* determine length of identifier, and hunt for it.                 */
    /* ---------------------------------------------------------------- */

    for (c = 0; c < md_max_reg; c++)
      if (md_list[c] != NULL &&
          strcasecmp (md_list[c]->ident, ident) == 0)
        return (md_list[c]);

    return (NULL);
}


/* -------------------------------------------------------------------- */
/* function    : md_register                                            */
/* description : register an instance of an algorithm                   */
/* -------------------------------------------------------------------- */
GLOBAL err_code
md_register (ident, register_func)
    string          ident;          /* algorithm identifier             */
    function        register_func;  /* function to setup info struct    */
{
    word            c;

    DEBUG_FUNCTION_ENTER("md_register");

    /* ---------------------------------------------------------------- */
    /* check to see if it is registered and we have space               */
    /* ---------------------------------------------------------------- */

    if (md_lookup_id(ident) != NULL)
      return (err_already_exist);

    /* ---------------------------------------------------------------- */
    /* if not, find free slot                                           */
    /* ---------------------------------------------------------------- */

    c = 0;
    while (c < md_max_reg && md_list[c] != NULL)
      if (++c == md_max_reg)
        return (err_no_space);

    /* ---------------------------------------------------------------- */
    /* create entry, and call initialisation function                   */
    /* ---------------------------------------------------------------- */

    md_list[c] = (md_info*) mem_get (sizeof(md_info), MEMOPT_CLEAR);
    strncpy (md_list[c]->ident, ident, ident_sz);

    return (*register_func)(md_list[c]);    /* return it's error code   */
}


/* -------------------------------------------------------------------- */
/* function    : md_deregister                                          */
/* description : deregister an instance of an algorithm                 */
/* -------------------------------------------------------------------- */
GLOBAL err_code
md_deregister (ident)
    string          ident;          /* algorithm identifier             */
{
    word            c;

    DEBUG_FUNCTION_ENTER("md_deregister");

    /* ---------------------------------------------------------------- */
    /* find the relevant slot and free the particular structure         */
    /* ---------------------------------------------------------------- */

    for (c = 0; c < md_max_reg; c++)
      if (md_list[c] != NULL &&
          strcasecmp (md_list[c]->ident, ident) == 0)
        {
          if (md_list[c]->deregister != NULL)
            (void)(*(md_list[c]->deregister))(md_list[c]);  /* ignore   */

          mem_free (md_list[c]);
          md_list[c] = NULL;

          return (err_ok);
        }

    return (err_not_found);
}


/* -------------------------------------------------------------------- */
/* function    : md_mod_open                                            */
/* description : initialise by registering all the message digests that */
/*               are to be linked in with this module                   */
/* -------------------------------------------------------------------- */
GLOBAL err_code
md_mod_open ()
{

    DEBUG_FUNCTION_ENTER("md_mod_open");

    /* ---------------------------------------------------------------- */
    /* clear enter list structure and then call to setup                */
    /* ---------------------------------------------------------------- */

    memset (md_list, 0, sizeof(md_list));

    /* ---------------------------------------------------------------- */
    /* register the already known ones -- insert yours here             */
    /* NOTE: they currently ignore return error codes                   */
    /* ---------------------------------------------------------------- */

    md_register ("md4", MD4_REGISTER);
    md_register ("shs", SHS_REGISTER);
    md_register ("md5", MD5_REGISTER);
    md_register ("haval", HAVAL_REGISTER);
#ifdef MD_INCLUDE_SNEFRU
    md_register ("snefru", SNEFRU_REGISTER);
#endif

    return (err_ok);
}

/* -------------------------------------------------------------------- */
/* function    : md_mod_close                                           */
/* description : shutdown the module, and release algorithms            */
/* -------------------------------------------------------------------- */
GLOBAL err_code
md_mod_close ()
{
    word            c;

    DEBUG_FUNCTION_ENTER("md_mod_close");

    /* ---------------------------------------------------------------- */
    /* deregister each one in turn                                      */
    /* ---------------------------------------------------------------- */

    for (c = 0; c < md_max_reg; c++)
      if (md_list[c] != NULL)
        {
          if (md_list[c]->deregister != NULL)
            (void)(*(md_list[c]->deregister))(md_list[c]);  /* ignore   */

          mem_free (md_list[c]);
          md_list[c] = NULL;
        }

    return (err_ok);
}


/* -------------------------------------------------------------------- */
/* function    : md_open                                                */
/* description : the first instance of creation of any context must     */
/*               come here                                              */
/* -------------------------------------------------------------------- */
GLOBAL err_code
md_open (req_ctx, ident)
    md_ctx*         req_ctx;        /* context to be initialised        */
    string          ident;          /* algorithm to use                 */
{

    DEBUG_FUNCTION_ENTER("md_open");

    /* ---------------------------------------------------------------- */
    /* check to make sure the request message digest is valid           */
    /* and then setup                                                   */
    /* ---------------------------------------------------------------- */

    req_ctx->info = md_lookup_id (ident);

    if (req_ctx->info == NULL)
      return (err_invalid_alg);

    /* ---------------------------------------------------------------- */
    /* call the internal creation routine for the digest                */
    /* ---------------------------------------------------------------- */

    req_ctx->priv = NULL;

    return  md_create (req_ctx);    /* return err_code from create      */
}


/* -------------------------------------------------------------------- */
/* function    : md_close                                               */
/* description : close down a context by freeing any information        */
/* -------------------------------------------------------------------- */
GLOBAL err_code
md_close (req_ctx)
    md_ctx*         req_ctx;        /* context to be close              */
{

    DEBUG_FUNCTION_ENTER("md_close");

    md_destroy (req_ctx);

    return (err_ok);
}


/* -------------------------------------------------------------------- */
/* function    : md_list_init                                           */
/* description : initialise a state variable for walking the list       */
/* -------------------------------------------------------------------- */
GLOBAL err_code
md_list_init (state_var)
    word*            state_var;     /* state variable                   */
{

    DEBUG_FUNCTION_ENTER("md_list_init");

    *state_var = 0;

    return (err_ok);
}


/* -------------------------------------------------------------------- */
/* function    : md_list_next                                           */
/* description : walk down the list                                     */
/* -------------------------------------------------------------------- */
GLOBAL err_code
md_list_next (state_var, ident)
    word*           state_var;      /* state variable                   */
    string          ident;          /* where to place ident             */
{

    DEBUG_FUNCTION_ENTER("md_list_next");

    /* ---------------------------------------------------------------- */
    /* basic strategy is to search from current position to a non empty */
    /* info structure, if the state_variable is out of the allowable    */
    /* range, an error is returned, and if nothing is found, an error   */
    /* is also returned. err_ok signifies successfull completion        */
    /* ---------------------------------------------------------------- */

    if (*state_var < 0 || *state_var >= md_max_reg)
      return (err_invalid_params);

    while (md_list[*state_var] == NULL && *state_var < md_max_reg)
      (*state_var)++;

    if (*state_var == md_max_reg)
      return (err_no_more);

    strcpy (ident, md_list[*state_var]->ident);

    (*state_var)++;

    return (err_ok);
}


/* -------------------------------------------------------------------- */

