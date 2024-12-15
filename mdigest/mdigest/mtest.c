
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

/* $Id: mfile.c%v 0.90 1994/02/22 20:51:46 matt Exp $
 */

/*
 *  File:         mtest.c
 *  Description:  test md4, md5, shs and (if added) other digests
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <mdigest.h>

/* -------------------------------------------------------------------- */

static string           digest_to_hex ();
static word             test_md4 ();
static word             test_md5 ();
static word             test_shs ();
static word             test_haval ();

/* -------------------------------------------------------------------- */

typedef struct
  {
    string              ident;
    word                (*test)();
  } test_func;

test_func               testable_mds[] =
  {
    { "md4",            test_md4 },
    { "md5",            test_md5 },
    { "shs",            test_shs },
    { "haval",          test_haval },
    { NULL,             NULL     },
  };

/* -------------------------------------------------------------------- */
/* function    : main                                                   */
/* -------------------------------------------------------------------- */
void
main (argc, argv)
    int             argc;
    char**          argv;
{
    word            c;
    word            s;
    md_ctx          ctx;
    char            ident[ident_sz+1];

    /* ---------------------------------------------------------------- */
    /* open the module and try to test whatever is available            */
    /* ---------------------------------------------------------------- */

    md_mod_open ();

    md_list_init (&s);
    while (md_list_next (&s, ident) == err_ok)
      {
        printf ("%-8s : ", ident);

        for (c = 0; testable_mds[c].ident != NULL &&
                    strcasecmp(ident, testable_mds[c].ident) != 0; c++)
          ;

        if (testable_mds[c].ident == NULL)
          {
            printf ("no test suite exists.\n");
            continue;
          }

        /* ------------------------------------------------------------ */
        /* create an association for that particular message digest     */
        /* ------------------------------------------------------------ */

        if (md_open (&ctx, ident) != err_ok)
          {
            printf ("internal error.\n");
            exit (1);
          }

        /* ------------------------------------------------------------ */
        /* call it, and then close the association                      */
        /* ------------------------------------------------------------ */

        c = (*(testable_mds[c].test))(&ctx);

        if (c == -1)
          printf ("test(s) failed !\n");
        else
          printf ("%d test%s, all successfull.\n", c, (c == 1) ?"":"s");

        md_close (&ctx);

      }

    exit (0);
}


/* -------------------------------------------------------------------- */
/* function    : digest_to_hex                                          */
/* description : format a digest into a hex string                      */
/* -------------------------------------------------------------------- */
static string
digest_to_hex (digest, sz)
    opaque          digest;         /* bytes of digest                  */
    word            sz;             /* # bytes in digest                */
{
    static char     hex_str[81];
    word            c;

    for (c = 0; c < sz; c++)
      sprintf (&hex_str[c<<1], "%02x", (digest[c] & 0xff));

    hex_str[c<<1] = '\0';
    return (hex_str);
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
/* place test routines/structures/info from here onwards ...            */
/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

typedef struct
  {
    string              test;
    string              result;
  } test_set;

char                    digest[64];
string                  hdigest;

/* -------------------------------------------------------------------- */
/* function    : check_test_set                                         */
/* description : check all items in a test_set                          */
/* -------------------------------------------------------------------- */
STATIC word
check_test_set (ctx, tset)
    md_ctx*         ctx;
    test_set        tset[];
{
    word            c;

    for (c = 0; tset[c].test != NULL; c++)
      {
        md_init (ctx);
        md_update (ctx, tset[c].test, strlen(tset[c].test));
        md_final (ctx, digest);

        hdigest = digest_to_hex (digest, md_hash_sz (ctx));

        if (strcmp (hdigest, tset[c].result) != 0)
          return -1;
      }

    return c;
}


/* -------------------------------------------------------------------- */
/* message-digest 4 test suite from RFC-1186                            */
/* -------------------------------------------------------------------- */

static test_set         md4_tests[] =
  {
    { "",
      "31d6cfe0d16ae931b73c59d7e0c089c0" },
    { "a",
      "bde52cb31de33e46245e05fbdbd6fb24" },
    { "abc",
      "a448017aaf21d8525fc10ae87aa6729d" },
    { "message digest",
      "d9130a8164549fe818874806e1c7014b" },
    { "abcdefghijklmnopqrstuvwxyz",
      "d79e1c308aa5bbcdeea8ed63df412da9" },
    { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
      "043f8582f241db351ce627e153e7f0e4" },
    { "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
      "e33b4ddc9c38f2199c3e7b164fcc0536" },
    { NULL,
      NULL },
  };

/* -------------------------------------------------------------------- */
/* function    : test_md4                                               */
/* description : run through the md4 test suite                         */
/* -------------------------------------------------------------------- */
STATIC word
test_md4 (ctx)
    md_ctx*         ctx;
{
    return  check_test_set (ctx, md4_tests);
}


/* -------------------------------------------------------------------- */
/* message-digest 5 test suite from RFC-1321                            */
/* -------------------------------------------------------------------- */

static test_set         md5_tests[] =
  {
    { "",
      "d41d8cd98f00b204e9800998ecf8427e" },
    { "a",
      "0cc175b9c0f1b6a831c399e269772661" },
    { "abc",
      "900150983cd24fb0d6963f7d28e17f72" },
    { "message digest",
      "f96b697d7cb7938d525a2f31aaf161d0" },
    { "abcdefghijklmnopqrstuvwxyz",
      "c3fcd3d76192e4007dfb496cca67e13b" },
    { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
      "d174ab98d277d9f5a5611c2c9f419d9f" },
    { "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
      "57edf4a22be3c955ac49da2e2107b67a" },
    { NULL,
      NULL },
  };

/* -------------------------------------------------------------------- */
/* function    : test_md5                                               */
/* description : run through the md5 test suite                         */
/* -------------------------------------------------------------------- */
STATIC word
test_md5 (ctx)
    md_ctx*         ctx;
{
   return  check_test_set (ctx, md5_tests);
}


/* -------------------------------------------------------------------- */
/* NIST shs test                                                        */
/* -------------------------------------------------------------------- */

static test_set         shs_tests[] =
  {
    { "abc",
      "0164b8a914cd2a5e74c4f7ff082c4d97f1edf880" },
    { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
      "d2516ee1acfa5baf33dfc1c471e438449ef134c8" },
    { NULL,
      NULL },
  };

/* -------------------------------------------------------------------- */
/* function    : test_shs                                               */
/* description : go through NIST shs tests                              */
/* -------------------------------------------------------------------- */
STATIC word
test_shs (ctx)
    md_ctx*         ctx;
{
    word            c;
    word            i;
    char            buf[1000];

    c = check_test_set (ctx, shs_tests);

    if (c == -1)
      return -1;

    /* ---------------------------------------------------------------- */
    /* do 1million 'a's.                                                */
    /* ---------------------------------------------------------------- */

    memset (buf, 'a', 1000);

    md_init (ctx);
    for (i = 0; i < 1000; i++)
      md_update (ctx, buf, 1000);
    md_final (ctx, digest);

    hdigest = digest_to_hex (digest, md_hash_sz (ctx));

    if (strcmp (hdigest, "3232affa48628a26653b5aaa44541fd90d690603") != 0)
      return -1;

    return ++c;
}


/* -------------------------------------------------------------------- */
/* function    : test_haval                                             */
/* description : do haval test                                          */
/* -- NOTE --  : this assumes the default pass3=,fptlen=128/160 setup!  */
/* -------------------------------------------------------------------- */
STATIC word
test_haval (ctx)
    md_ctx*         ctx;
{
    string          test_f128 = "";
    string          result_f128 = "1bdc556b29ad02ec09af8c66477f2a87";
    string          test_f160 = "a";
    string          result_f160 = "5e1610fced1d3adb0bb18e92ac2b11f0bd99d8ed";
    string          t, r;


    if (md_hash_sz (ctx) == (128>>3))
      {
        t = test_f128;
        r = result_f128;
      }
    else if (md_hash_sz (ctx) == (160>>3))
      {
        t = test_f160;
        r = result_f160;
      }
    else
      return -1;
      
    md_init (ctx);
    md_update (ctx, t, strlen (t));
    md_final (ctx, digest);

    hdigest = digest_to_hex (digest, md_hash_sz (ctx));

    if (strcmp (hdigest, r) != 0)
      return -1;

    return 1;
}

/* -------------------------------------------------------------------- */


