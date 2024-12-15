
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
 *  File:         mfile.c
 *  Description:  generate message digests for input files using
 *                specific digest algs
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <mdigest.h>


/* -------------------------------------------------------------------- */
/* variables                                                            */
/* -------------------------------------------------------------------- */

static char         md_opt[md_max_reg][ident_sz+1]; /* ident storage    */
word                md_opt_sz   = 0;    /* number of idents loaded      */
word                md_opt_u    = 0;    /* bitstring of md's in use     */

#define BIT_P(x)    (1L<<(x))

md_ctx              ctx_lst[md_max_reg];    /* contexts to be used      */
word                c;                      /* temp counter             */

#define BUF_LEN     (16384)         /* input buffer size                */
opaque              read_buf;       /* input buffer                     */

static void	    file_sig ();
static string	    digest_to_hex ();

/* -------------------------------------------------------------------- */
/* function    : show_usage                                             */
/* description : show command line usage                                */
/* -------------------------------------------------------------------- */
static void
show_usage (fn)
    string          fn;             /* argv[0]                          */
{
    word            i;

    fprintf (stderr, "Usage: %s ", fn);

    for (i = 0; i < md_opt_sz; i++)
      fprintf (stderr, "[-%s] ", md_opt[i]);

    fprintf (stderr, "file ...\n" );

    exit (1);
}


/* -------------------------------------------------------------------- */
/* function    : main                                                   */
/* -------------------------------------------------------------------- */
void
main (argc, argv)
    int             argc;
    char**          argv;
{
    int             ac;
    int             file_h;
    word            s;

    /* ---------------------------------------------------------------- */
    /* walk the list of preregistered message digests and store them    */
    /* locally                                                          */
    /* ---------------------------------------------------------------- */

    md_mod_open ();
    md_list_init (&s);
    while (md_list_next (&s, md_opt[md_opt_sz] ) == err_ok)
      md_opt_sz++;


    /* ---------------------------------------------------------------- */
    /* parse command line, looking for '-<md>'                          */
    /* ---------------------------------------------------------------- */

    for (ac = 1; ac < argc && argv[ac][0] == '-'; ac++)
      {
        for (c = 0; c < md_opt_sz &&
                    strncasecmp (&argv[ac][1], md_opt[c], 3) != 0; c++)
          ;

        if (c == md_opt_sz)         /* couldn't find the alg            */
          show_usage (argv[0]);

        md_opt_u |= BIT_P(c);
      }


    /* ---------------------------------------------------------------- */
    /* if nothing has been specified, then turn them all on             */
    /* ---------------------------------------------------------------- */

    if (md_opt_u == 0)
      md_opt_u = (BIT_P(md_opt_sz) - 1);


    /* ---------------------------------------------------------------- */
    /* initialise the context for each requested alg                    */
    /* ---------------------------------------------------------------- */

    for (c = 0; c < md_opt_sz; c++)
      if (md_opt_u & BIT_P(c))
        if (md_open (&ctx_lst[c], md_opt[c]) != err_ok)
          {
            md_opt_u &= ~BIT_P(c);
            fprintf (stderr, " - notice: problem initialising digest '%s'.\n",
                    md_opt[c]);
          }


    /* ---------------------------------------------------------------- */
    /* allocate the file buffer, then process each file or stdin if no  */
    /* files were passed on the command line                            */
    /* ---------------------------------------------------------------- */

    read_buf = mem_get(BUF_LEN , MEMOPT_NONE);

    if (ac == argc)
      {
        printf ("stdin : ");
        file_sig (fileno(stdin));
      }
    else
      while (ac < argc)
        {
          printf ("%s : ", argv[ac]);

          if ((file_h = open(argv[ac], O_RDONLY)) < 0)
            printf ("file not found.\n");
          else
            {
              file_sig (file_h);
              close (file_h);
            }

          ac++;
        }

    /* ---------------------------------------------------------------- */
    /* destroy buffers, close module and exit                           */
    /* ---------------------------------------------------------------- */

    mem_free (read_buf);

    for (c = 0; c < md_opt_sz; c++)
      if (md_opt_u & BIT_P(c))
        (void)md_close (&ctx_lst[c]);

    md_mod_close ();

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
/* function    : file_sig                                               */
/* description : perform signature on the given file                    */
/* -------------------------------------------------------------------- */
static void
file_sig (file_h)
    int             file_h;         /* file handle                      */
{
    int             r;
    char            digest[48];
    word32          fs = 0;

    /* ---------------------------------------------------------------- */
    /* initialise all md contexts                                       */
    /* ---------------------------------------------------------------- */

    for (c = 0; c < md_opt_sz; c++)
      if (md_opt_u & BIT_P(c))
        md_init (&ctx_lst[c]);


    /* ---------------------------------------------------------------- */
    /* pull in buffers from file, updating md on each                   */
    /* ---------------------------------------------------------------- */

    do
      {
        if ((r = read(file_h, read_buf, BUF_LEN)) <= 0)
          break;

        for (c = 0; c < md_opt_sz; c++)
          if (md_opt_u & BIT_P(c))
            md_update (&ctx_lst[c], read_buf, r);

        fs += r;

      } while (r == BUF_LEN);

    /* ---------------------------------------------------------------- */
    /* display final tally                                              */
    /* ---------------------------------------------------------------- */

    printf ("%lu bytes\n", fs);

    for (c = 0; c < md_opt_sz; c++)
      if (md_opt_u & BIT_P(c))
        {
          md_final (&ctx_lst[c], digest);

          printf ("  %-8s : %s\n", md_opt[c],
             digest_to_hex(digest, md_hash_sz(&ctx_lst[c])));
        }
}

/* -------------------------------------------------------------------- */
