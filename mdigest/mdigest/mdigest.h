
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

/* $Id: mdigest.h%v 0.90 1994/02/22 20:37:24 matt Exp $
 */

/*
 *  File:         mdigest.h
 *  Description:  Message digest header file
 */

#ifndef _MDIGEST_H_
#define _MDIGEST_H_

/* -------------------------------------------------------------------- */
/* header files                                                         */
/* -------------------------------------------------------------------- */

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>

/* -------------------------------------------------------------------- */
/* usual low level type castings                                        */
/* -------------------------------------------------------------------- */

typedef unsigned char   word8;      /* defined as per bit size          */
typedef unsigned short  word16;
typedef unsigned long   word32;

typedef int             word;       /* ''generic'' integer              */

typedef unsigned char*  opaque;     /* arbitrary data segments          */
typedef char*           string;     /* character strings                */

typedef void            (*handler)();   /* void function handler        */

/* -------------------------------------------------------------------- */
/* nearly all functions return error codes as defined below             */
/* -------------------------------------------------------------------- */

typedef enum
  {
    err_ok              = 0,        /* no error occured                 */
    err_no_memory       = 1,        /* memory allocation failed         */
    err_invalid_params  = 2,        /* invalid parameters passed        */
    err_file_not_found  = 3,        /* can't find a requested filename  */
    err_cant_open_file  = 4,        /* can't open a requested filename  */
    err_invalid_alg     = 5,        /* unknown algorithm from somewhere */
    err_already_exist   = 6,        /* something already exists         */
    err_no_space        = 7,        /* internal table/etc is full       */
    err_not_found       = 8,        /* something was not found          */
    err_no_more         = 9,        /* no more is available             */
    err_not_init        = 10,       /* not initialised                  */
    err_unknown         = 255,      /* unknown error                    */
  } err_code;

typedef err_code        (*function)();  /* general function handler     */


/* -------------------------------------------------------------------- */
/* macros for general purpose use                                       */
/* -------------------------------------------------------------------- */

#ifndef min
# define min(a, b)      ( ((a) <= (b)) ? (a) : (b) )
# define max(a, b)      ( ((a) >= (b)) ? (a) : (b) )
#endif

#ifndef TRUE
# define TRUE           (1)
# define FALSE          (0)
#endif

#define BIT_POS(x)      (1L<<(x))   /* specific bit position            */
#define ident_sz        (16)        /* identifer sizes in the system    */

#define GLOBAL                      /* functions for rest of the world  */
#define STATIC          static      /* local to specific file           */
#define reg             register    /* quick defn for register vars     */


/* -------------------------------------------------------------------- */
/* simple bitstring class and operators                                 */
/* -------------------------------------------------------------------- */

typedef word32          bitstring;

#define bit_max         (sizeof(bitstring)<<3)
#define bit_set(s, b)   ((s) |= (1L<<(b))
#define bit_clr(s, b)   ((s) &= ~(1L<<(b))
#define bit_zero(s)     (s) = ((bitstring)0L)
#define bit_inv(s)      (s) ^= ((bitstring)0xffffffff)
#define bit_chk(s, b)   ((((s) &= ~(1L<<(b))) == 0) ? FALSE : TRUE)

#define bit_pos(b)      (1L<<(b))
#define bit_empty       (0L)


/* -------------------------------------------------------------------- */
/* debug facilities                                                     */
/* -------------------------------------------------------------------- */

#define DEBUG_FUNCTION_ENTER(f)


/* -------------------------------------------------------------------- */
/* memory.c requirements                                                */
/* -------------------------------------------------------------------- */

#define MEMOPT_NONE     bit_empty   /* no options                       */
#define MEMOPT_CLEAR    bit_pos(0)  /* clear segment before use         */


/* -------------------------------------------------------------------- */
/* max number of registerable hash functions                            */
/* -------------------------------------------------------------------- */

#define md_max_reg          (8)


/* -------------------------------------------------------------------- */
/* at registration, each algorithm provides function handlers and other */
/* information                                                          */
/* -------------------------------------------------------------------- */

typedef struct
  {
    char                ident[ident_sz+1];  /* id string                */

    word                block_sz;   /* input block size in bytes        */
    word                hash_sz;    /* output has size in bytes         */

    function            create;     /* creation handler                 */
    function            destroy;    /* destruction                      */
    function            deregister; /* deregistration                   */

    handler             init;       /* init for start of hash           */
    handler             update;     /* update for hash                  */
    handler             final;      /* final and calc digest            */

  } md_info;

/* -------------------------------------------------------------------- */
/* each association with the module uses its own context:               */
/* -------------------------------------------------------------------- */

typedef struct
  {
    md_info             *info;      /* ptr to above info block          */
    opaque              priv;       /* private structure                */

  } md_ctx;


#define md_create(c)        (*((c)->info->create))(c)   /* not for user */
#define md_destroy(c)       (*((c)->info->destroy))(c)  /* not for user */

#define md_init(c)          (*((c)->info->init))((c)->priv)       /* u  */
#define md_update(c,d,l)    (*((c)->info->update))((c)->priv,d,l) /* u  */
#define md_final(c,o)       (*((c)->info->final))((c)->priv,o)    /* u  */

#define md_hash_sz(c)       ((c)->info->hash_sz)
#define md_block_sz(c)      ((c)->info->block_sz)
#define md_id(c)            ((c)->info->ident)


/* -------------------------------------------------------------------- */
/* prototypes -- k'n'r */

opaque      mem_get ();
void        mem_free ();
string      sacs_err_msg ();
md_info*    md_lookup_id ();
err_code    md_register ();
err_code    md_deregister ();
err_code    md_mod_open ();
err_code    md_mod_close ();
err_code    md_open ();
err_code    md_close ();
err_code    md_list_init ();
err_code    md_list_next ();

/* -------------------------------------------------------------------- */
#endif /*_MDIGEST_H_*/

