#ifndef lint
static char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93 (BSDI)";
#endif
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYEMPTY (-1)
#define YYLEX yylex()
#define yyclearin (yychar=YYEMPTY)
#define yyerrok (yyerrflag=0)
#define YYRECOVERING (yyerrflag!=0)
#define YYPREFIX "yy"
#line 2 "grammar.y"
/*	BSDI	grammar.y,v 2.1 1995/02/03 18:11:55 polk Exp	*/

/*
 * Copyright (c) 1988, 1989, 1990, 1991, 1992, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */
#ifndef lint
static char rcsid[] =
    "@(#) /master/usr.sbin/tcpdump/libpcap/grammar.y,v 2.1 1995/02/03 18:11:55 polk Exp (LBL)";
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <net/if.h>
#ifndef __bsdi__
#include <net/bpf.h>
#endif

#include <netinet/in.h>
#include <netinet/if_ether.h>

#include <stdio.h>
#include <pcap.h>
#include <pcap-namedb.h>

#include "gencode.h"

#define QSET(q, p, d, a) (q).proto = (p),\
			 (q).dir = (d),\
			 (q).addr = (a)

int n_errors = 0;

static struct qual qerr = { Q_UNDEF, Q_UNDEF, Q_UNDEF, Q_UNDEF };

static void
yyerror(char *msg)
{
	++n_errors;
	bpf_error(msg);
	/* NOTREACHED */
}

#ifndef YYBISON
pcap_parse()
{
	return (yyparse());
}
#endif

#line 73 "grammar.y"
typedef union {
	int i;
	u_long h;
	u_char *e;
	char *s;
	struct stmt *stmt;
	struct arth *a;
	struct {
		struct qual q;
		struct block *b;
	} blk;
	struct block *rblk;
} YYSTYPE;
#line 98 "y.tab.c"
#define DST 257
#define SRC 258
#define HOST 259
#define GATEWAY 260
#define NET 261
#define PORT 262
#define LESS 263
#define GREATER 264
#define PROTO 265
#define BYTE 266
#define ARP 267
#define RARP 268
#define IP 269
#define TCP 270
#define UDP 271
#define ICMP 272
#define DECNET 273
#define LAT 274
#define MOPRC 275
#define MOPDL 276
#define TK_BROADCAST 277
#define TK_MULTICAST 278
#define NUM 279
#define INBOUND 280
#define OUTBOUND 281
#define LINK 282
#define GEQ 283
#define LEQ 284
#define NEQ 285
#define ID 286
#define EID 287
#define HID 288
#define LSH 289
#define RSH 290
#define LEN 291
#define OR 292
#define AND 293
#define UMINUS 294
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,   24,    1,    1,    1,    1,    1,   20,   21,
    2,    2,    2,    3,    3,    3,    3,   23,   22,    4,
    4,    4,    7,    7,    5,    5,    8,    8,    8,    8,
    8,    6,    6,    6,    6,    6,    6,    9,    9,   10,
   10,   10,   10,   10,   10,   11,   11,   11,   12,   16,
   16,   16,   16,   16,   16,   16,   16,   16,   16,   16,
   25,   25,   25,   25,   25,   25,   25,   18,   18,   18,
   19,   19,   19,   13,   13,   14,   14,   14,   14,   14,
   14,   14,   14,   14,   14,   14,   14,   14,   15,   15,
   15,   15,   15,   17,   17,
};
short yylen[] = {                                         2,
    2,    1,    0,    1,    3,    3,    3,    3,    1,    1,
    1,    1,    3,    1,    1,    1,    2,    1,    1,    1,
    3,    3,    1,    1,    1,    2,    3,    2,    2,    2,
    2,    2,    3,    1,    3,    3,    1,    1,    0,    1,
    1,    3,    3,    3,    3,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    2,    2,    2,    2,    4,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    4,    6,    3,    3,    3,
    3,    3,    3,    3,    3,    2,    3,    1,    1,    1,
    1,    1,    1,    1,    3,
};
short yydefred[] = {                                      3,
    0,    0,    0,    0,    0,   52,   53,   51,   54,   55,
   56,   57,   58,   60,   59,   94,   66,   67,   50,   88,
   18,    0,   19,    0,    4,   25,    0,    0,    0,   75,
    0,   74,    0,    0,   37,   63,   64,    0,   86,    0,
    0,   10,    9,    0,    0,   14,   16,   15,   32,   11,
   12,    0,    0,    0,    0,   46,   49,   47,   48,   30,
   61,   62,    0,   29,   31,   69,   71,   73,    0,    0,
    0,    0,    0,    0,    0,    0,   68,   70,   72,    0,
    0,    0,    0,    0,    0,   26,   90,   89,   92,   93,
   91,    0,    0,    6,    5,    0,    0,    0,    8,    7,
   20,    0,    0,    0,    0,   17,    0,    0,    0,    0,
   27,    0,    0,    0,    0,    0,    0,   80,   81,    0,
    0,    0,   33,   87,   95,   65,    0,   13,    0,    0,
    0,   43,   45,   42,   44,   76,    0,   21,   22,    0,
   77,
};
short yydgoto[] = {                                       1,
   83,  106,   50,  102,   25,   26,  103,   27,   28,   63,
   64,   65,   29,   30,   92,   40,   32,   80,   81,   44,
   45,   41,   53,    2,   35,
};
short yysindex[] = {                                      0,
    0,  -26, -274, -270, -268,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  247,    0, -272,    0,    0,  -11,  336,  334,    0,
  -74,    0,  -26,  -26,    0,    0,    0,  -28,    0,  -74,
  247,    0,    0,  138,  138,    0,    0,    0,    0,    0,
    0,  -11,  -11, -211, -199,    0,    0,    0,    0,    0,
    0,    0, -221,    0,    0,    0,    0,    0,  247,  247,
  247,  247,  247,  247,  247,  247,    0,    0,    0,  247,
  247,  247,  -41,  -18,  -16,    0,    0,    0,    0,    0,
    0, -252,  344,    0,    0,    0,  138,  138,    0,    0,
    0,    4, -272,  -16,  -22,    0, -222, -206, -203, -197,
    0,  115,  115,  310,  409,  -39,  -39,    0,    0,  344,
  344,  450,    0,    0,    0,    0,  -16,    0,  -11,  -11,
  -16,    0,    0,    0,    0,    0, -214,    0,    0,  -35,
    0,
};
short yyrindex[] = {                                      0,
    0,    2,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   68,    0,    0,    0,    0,    0,    0,
  106,    0,  381,  381,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  381,  381,    0,    0,    0,    0,    0,
    0,    0,    0,  184,  188,    0,    0,    0,    0,    0,
    0,    0,   -5,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  417,  442,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    1,  381,  381,    0,    0,
    0, -170,    0, -159,    0,    0,    0,    0,    0,    0,
    0,   37,   51,   77,   66,   12,   26,    0,    0,    6,
   15,    0,    0,    0,    0,    0,  149,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,
};
short yygindex[] = {                                      0,
   74,  -14,  -48,    0,   87,    0,    0,    0,    0,    0,
   16,    0,  508,  -17,    0,   57,  507,    0,    0,  -20,
   -3,  523,  108,    0,    0,
};
#define YYTABLESIZE 740
short yytable[] = {                                     123,
   12,    2,   75,  101,   36,   35,   21,   76,   37,   88,
   38,   78,   49,   23,   36,   84,   82,   23,   22,   42,
   43,   21,  124,   84,  125,   79,  126,   28,   23,   94,
   99,   91,   90,   89,   28,  132,   84,   56,   74,   58,
   59,   12,   74,   74,  128,   74,   35,   74,  101,   78,
   85,  133,   78,  134,   78,   36,   78,  141,   31,  135,
   74,   74,   74,   79,  140,   82,   79,    1,   79,   78,
   79,   78,   78,   78,   84,   24,   83,   84,  111,   84,
  107,  108,  129,   79,    0,   79,   79,   79,   85,   31,
   31,   85,  109,  110,   84,   87,   84,   84,   84,  130,
   31,   31,    0,   82,   78,   34,   82,    0,   85,   34,
   85,   85,   85,    0,  138,  139,    0,   83,   79,    0,
   86,   24,   24,   82,   74,   82,   82,   82,    0,   84,
   95,  100,   23,   23,   83,   78,   83,   83,   83,    0,
   34,   34,    0,   85,    0,    0,   34,    0,    0,   79,
    0,   98,   98,   31,   31,    0,   75,   73,   82,   74,
   84,   76,    0,    0,    0,    0,    0,    0,    0,   83,
   21,    0,    0,    0,   85,    0,    0,   23,    0,    0,
    0,    0,   22,    0,   86,    0,   74,    0,    0,   82,
   74,   74,    0,   74,    0,   74,    0,    0,    0,    0,
   83,    0,    0,    0,   98,   98,    0,    0,   74,   74,
   74,    0,    0,    0,    0,    0,   41,    0,    0,    0,
   40,    0,    0,   41,    0,    0,    0,   40,    0,    0,
    0,    0,    0,    0,    0,    0,    3,    4,    0,    5,
    6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
   42,   43,   16,   17,   18,   19,   16,    0,   39,   39,
   39,   39,   39,   39,   20,    0,   39,   16,    0,    0,
    0,    0,   74,   28,   46,   47,   48,    0,   39,   39,
   28,   28,   28,   74,   74,   74,   23,    0,    0,   74,
   74,   22,   12,   12,   78,   78,   78,   35,   35,    0,
   78,   78,    0,   78,   78,    0,   36,   36,   79,   79,
   79,    0,    0,    0,   79,   79,    0,   79,   79,   84,
   84,   84,    0,    0,    0,   84,   84,    0,   84,   84,
    0,    0,    0,   85,   85,   85,    0,    0,    0,   85,
   85,    0,   85,   85,    0,    0,    0,   72,   82,   82,
   82,   75,   73,    0,   74,    0,   76,   82,   82,   83,
   83,   83,   38,   38,   38,   38,   38,   38,   83,   83,
   38,   72,    0,    0,    0,   75,   73,    0,   74,    0,
   76,   72,   38,   38,    0,   75,   73,    0,   74,    0,
   76,    0,    0,   79,   78,   77,    0,   34,   34,    0,
    3,    4,    0,    5,    6,    7,    8,    9,   10,   11,
   12,   13,   14,   15,    0,    0,   16,   17,   18,   19,
    0,    0,    0,   46,   47,   48,    0,    0,   20,    0,
    0,   74,   74,   74,    0,    0,    0,   74,   74,    0,
   23,   23,   41,    0,   41,   41,   40,    0,   40,   40,
   75,   73,    0,   74,   75,   76,    0,   71,   75,   75,
    0,   75,   41,   75,    0,    0,   40,   71,    0,   41,
   41,   41,    0,   40,   40,   40,   75,   75,   75,   74,
    0,    0,    0,   74,   74,    0,   74,   72,   74,    0,
    0,   75,   73,    0,   74,    0,   76,    0,    0,    0,
    0,   74,   74,   74,    0,    0,    0,  137,    0,    0,
    0,    0,    0,    6,    7,    8,    9,   10,   11,   12,
   13,   14,   15,    0,   33,   16,    0,    0,   19,   39,
    0,    0,    0,   51,    0,    0,    0,   20,    0,   85,
   75,    0,  136,    0,    0,    0,    0,   85,   93,   52,
   96,   96,    0,    0,    0,   33,   33,    0,  104,   51,
    0,    0,    0,    0,    0,   74,   97,   97,    0,    0,
    0,    0,    0,   71,  105,   52,  112,  113,  114,  115,
  116,  117,  118,  119,    0,    0,    0,  120,  121,  122,
    0,    0,   54,   55,   56,   57,   58,   59,   69,   70,
   60,    0,    0,  127,   96,    0,    0,    0,    0,    0,
    0,  131,   61,   62,    0,    0,   66,   67,   68,   33,
   97,    0,   69,   70,    0,    0,    0,  105,    0,    0,
    0,    0,   69,   70,    0,   51,   51,   39,   39,   39,
   39,   39,   39,    0,    0,   39,    0,    0,    0,    0,
    0,   52,   52,    0,    0,    0,    0,   39,   39,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   69,   70,   75,
   75,   75,    0,    0,    0,   75,   75,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   74,   74,   74,    0,    0,    0,
   74,   74,    0,    0,    0,    0,    0,    0,   69,   70,
};
short yycheck[] = {                                      41,
    0,    0,   42,   52,  279,    0,   33,   47,  279,   38,
  279,    0,   27,   40,    0,   33,   91,   40,   45,  292,
  293,   33,   41,   41,   41,    0,  279,   33,   40,   44,
   45,   60,   61,   62,   40,  258,    0,  259,   38,  261,
  262,   41,   42,   43,   41,   45,   41,   47,   97,   38,
    0,  258,   41,  257,   43,   41,   45,   93,    2,  257,
   60,   61,   62,   38,  279,    0,   41,    0,   43,   58,
   45,   60,   61,   62,   38,    2,    0,   41,   63,   97,
  292,  293,  103,   58,   -1,   60,   61,   62,   38,   33,
   34,   41,  292,  293,   58,  124,   60,   61,   62,  103,
   44,   45,   -1,   38,   93,    0,   41,   -1,   58,    2,
   60,   61,   62,   -1,  129,  130,   -1,   41,   93,   -1,
   34,  292,  293,   58,  124,   60,   61,   62,   -1,   93,
   44,   45,  292,  293,   58,  124,   60,   61,   62,   -1,
   33,   34,   -1,   93,   -1,   -1,   41,   -1,   -1,  124,
   -1,   44,   45,   97,   98,   -1,   42,   43,   93,   45,
  124,   47,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   93,
   33,   -1,   -1,   -1,  124,   -1,   -1,   40,   -1,   -1,
   -1,   -1,   45,   -1,   98,   -1,   38,   -1,   -1,  124,
   42,   43,   -1,   45,   -1,   47,   -1,   -1,   -1,   -1,
  124,   -1,   -1,   -1,   97,   98,   -1,   -1,   60,   61,
   62,   -1,   -1,   -1,   -1,   -1,   33,   -1,   -1,   -1,
   33,   -1,   -1,   40,   -1,   -1,   -1,   40,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,   -1,  266,
  267,  268,  269,  270,  271,  272,  273,  274,  275,  276,
  292,  293,  279,  280,  281,  282,  279,   -1,  257,  258,
  259,  260,  261,  262,  291,   -1,  265,  279,   -1,   -1,
   -1,   -1,  124,  279,  286,  287,  288,   -1,  277,  278,
  286,  287,  288,  283,  284,  285,   40,   -1,   -1,  289,
  290,   45,  292,  293,  283,  284,  285,  292,  293,   -1,
  289,  290,   -1,  292,  293,   -1,  292,  293,  283,  284,
  285,   -1,   -1,   -1,  289,  290,   -1,  292,  293,  283,
  284,  285,   -1,   -1,   -1,  289,  290,   -1,  292,  293,
   -1,   -1,   -1,  283,  284,  285,   -1,   -1,   -1,  289,
  290,   -1,  292,  293,   -1,   -1,   -1,   38,  283,  284,
  285,   42,   43,   -1,   45,   -1,   47,  292,  293,  283,
  284,  285,  257,  258,  259,  260,  261,  262,  292,  293,
  265,   38,   -1,   -1,   -1,   42,   43,   -1,   45,   -1,
   47,   38,  277,  278,   -1,   42,   43,   -1,   45,   -1,
   47,   -1,   -1,   60,   61,   62,   -1,  292,  293,   -1,
  263,  264,   -1,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,  276,   -1,   -1,  279,  280,  281,  282,
   -1,   -1,   -1,  286,  287,  288,   -1,   -1,  291,   -1,
   -1,  283,  284,  285,   -1,   -1,   -1,  289,  290,   -1,
  292,  293,  259,   -1,  261,  262,  259,   -1,  261,  262,
   42,   43,   -1,   45,   38,   47,   -1,  124,   42,   43,
   -1,   45,  279,   47,   -1,   -1,  279,  124,   -1,  286,
  287,  288,   -1,  286,  287,  288,   60,   61,   62,   38,
   -1,   -1,   -1,   42,   43,   -1,   45,   38,   47,   -1,
   -1,   42,   43,   -1,   45,   -1,   47,   -1,   -1,   -1,
   -1,   60,   61,   62,   -1,   -1,   -1,   58,   -1,   -1,
   -1,   -1,   -1,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,   -1,    2,  279,   -1,   -1,  282,   22,
   -1,   -1,   -1,   27,   -1,   -1,   -1,  291,   -1,   33,
  124,   -1,   93,   -1,   -1,   -1,   -1,   41,   41,   27,
   44,   45,   -1,   -1,   -1,   33,   34,   -1,   52,   53,
   -1,   -1,   -1,   -1,   -1,  124,   44,   45,   -1,   -1,
   -1,   -1,   -1,  124,   52,   53,   69,   70,   71,   72,
   73,   74,   75,   76,   -1,   -1,   -1,   80,   81,   82,
   -1,   -1,  257,  258,  259,  260,  261,  262,  289,  290,
  265,   -1,   -1,   97,   98,   -1,   -1,   -1,   -1,   -1,
   -1,  105,  277,  278,   -1,   -1,  283,  284,  285,   97,
   98,   -1,  289,  290,   -1,   -1,   -1,  105,   -1,   -1,
   -1,   -1,  289,  290,   -1,  129,  130,  257,  258,  259,
  260,  261,  262,   -1,   -1,  265,   -1,   -1,   -1,   -1,
   -1,  129,  130,   -1,   -1,   -1,   -1,  277,  278,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  289,  290,  283,
  284,  285,   -1,   -1,   -1,  289,  290,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  283,  284,  285,   -1,   -1,   -1,
  289,  290,   -1,   -1,   -1,   -1,   -1,   -1,  289,  290,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 294
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'!'",0,0,0,0,"'&'",0,"'('","')'","'*'","'+'",0,"'-'",0,"'/'",0,0,0,0,0,0,0,0,0,
0,"':'",0,"'<'","'='","'>'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,"'['",0,"']'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'|'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"DST","SRC","HOST","GATEWAY","NET","PORT","LESS",
"GREATER","PROTO","BYTE","ARP","RARP","IP","TCP","UDP","ICMP","DECNET","LAT",
"MOPRC","MOPDL","TK_BROADCAST","TK_MULTICAST","NUM","INBOUND","OUTBOUND","LINK",
"GEQ","LEQ","NEQ","ID","EID","HID","LSH","RSH","LEN","OR","AND","UMINUS",
};
char *yyrule[] = {
"$accept : prog",
"prog : null expr",
"prog : null",
"null :",
"expr : term",
"expr : expr and term",
"expr : expr and id",
"expr : expr or term",
"expr : expr or id",
"and : AND",
"or : OR",
"id : nid",
"id : pnum",
"id : paren pid ')'",
"nid : ID",
"nid : HID",
"nid : EID",
"nid : not id",
"not : '!'",
"paren : '('",
"pid : nid",
"pid : qid and id",
"pid : qid or id",
"qid : pnum",
"qid : pid",
"term : rterm",
"term : not term",
"head : pqual dqual aqual",
"head : pqual dqual",
"head : pqual aqual",
"head : pqual PROTO",
"head : pqual ndaqual",
"rterm : head id",
"rterm : paren expr ')'",
"rterm : pname",
"rterm : arth relop arth",
"rterm : arth irelop arth",
"rterm : other",
"pqual : pname",
"pqual :",
"dqual : SRC",
"dqual : DST",
"dqual : SRC OR DST",
"dqual : DST OR SRC",
"dqual : SRC AND DST",
"dqual : DST AND SRC",
"aqual : HOST",
"aqual : NET",
"aqual : PORT",
"ndaqual : GATEWAY",
"pname : LINK",
"pname : IP",
"pname : ARP",
"pname : RARP",
"pname : TCP",
"pname : UDP",
"pname : ICMP",
"pname : DECNET",
"pname : LAT",
"pname : MOPDL",
"pname : MOPRC",
"other : pqual TK_BROADCAST",
"other : pqual TK_MULTICAST",
"other : LESS NUM",
"other : GREATER NUM",
"other : BYTE NUM byteop NUM",
"other : INBOUND",
"other : OUTBOUND",
"relop : '>'",
"relop : GEQ",
"relop : '='",
"irelop : LEQ",
"irelop : '<'",
"irelop : NEQ",
"arth : pnum",
"arth : narth",
"narth : pname '[' arth ']'",
"narth : pname '[' arth ':' NUM ']'",
"narth : arth '+' arth",
"narth : arth '-' arth",
"narth : arth '*' arth",
"narth : arth '/' arth",
"narth : arth '&' arth",
"narth : arth '|' arth",
"narth : arth LSH arth",
"narth : arth RSH arth",
"narth : '-' arth",
"narth : paren narth ')'",
"narth : LEN",
"byteop : '&'",
"byteop : '|'",
"byteop : '<'",
"byteop : '>'",
"byteop : '='",
"pnum : NUM",
"pnum : paren pnum ')'",
};
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#define yystacksize YYSTACKSIZE
#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;
    extern char *getenv();

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if (yyn = yydefred[yystate]) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = YYLEX) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#ifdef lint
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
#line 122 "grammar.y"
{
	finish_parse(yyvsp[0].blk.b);
}
break;
case 3:
#line 127 "grammar.y"
{ yyval.blk.q = qerr; }
break;
case 5:
#line 130 "grammar.y"
{ gen_and(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
break;
case 6:
#line 131 "grammar.y"
{ gen_and(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
break;
case 7:
#line 132 "grammar.y"
{ gen_or(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
break;
case 8:
#line 133 "grammar.y"
{ gen_or(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
break;
case 9:
#line 135 "grammar.y"
{ yyval.blk = yyvsp[-1].blk; }
break;
case 10:
#line 137 "grammar.y"
{ yyval.blk = yyvsp[-1].blk; }
break;
case 12:
#line 140 "grammar.y"
{ yyval.blk.b = gen_ncode((u_long)yyvsp[0].i,
						   yyval.blk.q = yyvsp[-1].blk.q); }
break;
case 13:
#line 142 "grammar.y"
{ yyval.blk = yyvsp[-1].blk; }
break;
case 14:
#line 144 "grammar.y"
{ yyval.blk.b = gen_scode(yyvsp[0].s, yyval.blk.q = yyvsp[-1].blk.q); }
break;
case 15:
#line 145 "grammar.y"
{
				  /* Decide how to parse HID based on proto */
				  yyval.blk.q = yyvsp[-1].blk.q;
				  switch (yyval.blk.q.proto) {
				  case Q_DECNET:
					yyval.blk.b =
					    gen_ncode(__pcap_atodn((char *)yyvsp[0].h),
					    yyval.blk.q);
					break;
				  default:
					yyval.blk.b =
					    gen_ncode(__pcap_atoin((char *)yyvsp[0].h),
					    yyval.blk.q);
					break;
				  }
				}
break;
case 16:
#line 161 "grammar.y"
{ yyval.blk.b = gen_ecode(yyvsp[0].e, yyval.blk.q = yyvsp[-1].blk.q); }
break;
case 17:
#line 162 "grammar.y"
{ gen_not(yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
break;
case 18:
#line 164 "grammar.y"
{ yyval.blk = yyvsp[-1].blk; }
break;
case 19:
#line 166 "grammar.y"
{ yyval.blk = yyvsp[-1].blk; }
break;
case 21:
#line 169 "grammar.y"
{ gen_and(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
break;
case 22:
#line 170 "grammar.y"
{ gen_or(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
break;
case 23:
#line 172 "grammar.y"
{ yyval.blk.b = gen_ncode((u_long)yyvsp[0].i,
						   yyval.blk.q = yyvsp[-1].blk.q); }
break;
case 26:
#line 177 "grammar.y"
{ gen_not(yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
break;
case 27:
#line 179 "grammar.y"
{ QSET(yyval.blk.q, yyvsp[-2].i, yyvsp[-1].i, yyvsp[0].i); }
break;
case 28:
#line 180 "grammar.y"
{ QSET(yyval.blk.q, yyvsp[-1].i, yyvsp[0].i, Q_DEFAULT); }
break;
case 29:
#line 181 "grammar.y"
{ QSET(yyval.blk.q, yyvsp[-1].i, Q_DEFAULT, yyvsp[0].i); }
break;
case 30:
#line 182 "grammar.y"
{ QSET(yyval.blk.q, yyvsp[-1].i, Q_DEFAULT, Q_PROTO); }
break;
case 31:
#line 183 "grammar.y"
{ QSET(yyval.blk.q, yyvsp[-1].i, Q_DEFAULT, yyvsp[0].i); }
break;
case 32:
#line 185 "grammar.y"
{ yyval.blk = yyvsp[0].blk; }
break;
case 33:
#line 186 "grammar.y"
{ yyval.blk.b = yyvsp[-1].blk.b; yyval.blk.q = yyvsp[-2].blk.q; }
break;
case 34:
#line 187 "grammar.y"
{ yyval.blk.b = gen_proto_abbrev(yyvsp[0].i); yyval.blk.q = qerr; }
break;
case 35:
#line 188 "grammar.y"
{ yyval.blk.b = gen_relation(yyvsp[-1].i, yyvsp[-2].a, yyvsp[0].a, 0);
				  yyval.blk.q = qerr; }
break;
case 36:
#line 190 "grammar.y"
{ yyval.blk.b = gen_relation(yyvsp[-1].i, yyvsp[-2].a, yyvsp[0].a, 1);
				  yyval.blk.q = qerr; }
break;
case 37:
#line 192 "grammar.y"
{ yyval.blk.b = yyvsp[0].rblk; yyval.blk.q = qerr; }
break;
case 39:
#line 196 "grammar.y"
{ yyval.i = Q_DEFAULT; }
break;
case 40:
#line 199 "grammar.y"
{ yyval.i = Q_SRC; }
break;
case 41:
#line 200 "grammar.y"
{ yyval.i = Q_DST; }
break;
case 42:
#line 201 "grammar.y"
{ yyval.i = Q_OR; }
break;
case 43:
#line 202 "grammar.y"
{ yyval.i = Q_OR; }
break;
case 44:
#line 203 "grammar.y"
{ yyval.i = Q_AND; }
break;
case 45:
#line 204 "grammar.y"
{ yyval.i = Q_AND; }
break;
case 46:
#line 207 "grammar.y"
{ yyval.i = Q_HOST; }
break;
case 47:
#line 208 "grammar.y"
{ yyval.i = Q_NET; }
break;
case 48:
#line 209 "grammar.y"
{ yyval.i = Q_PORT; }
break;
case 49:
#line 212 "grammar.y"
{ yyval.i = Q_GATEWAY; }
break;
case 50:
#line 214 "grammar.y"
{ yyval.i = Q_LINK; }
break;
case 51:
#line 215 "grammar.y"
{ yyval.i = Q_IP; }
break;
case 52:
#line 216 "grammar.y"
{ yyval.i = Q_ARP; }
break;
case 53:
#line 217 "grammar.y"
{ yyval.i = Q_RARP; }
break;
case 54:
#line 218 "grammar.y"
{ yyval.i = Q_TCP; }
break;
case 55:
#line 219 "grammar.y"
{ yyval.i = Q_UDP; }
break;
case 56:
#line 220 "grammar.y"
{ yyval.i = Q_ICMP; }
break;
case 57:
#line 221 "grammar.y"
{ yyval.i = Q_DECNET; }
break;
case 58:
#line 222 "grammar.y"
{ yyval.i = Q_LAT; }
break;
case 59:
#line 223 "grammar.y"
{ yyval.i = Q_MOPDL; }
break;
case 60:
#line 224 "grammar.y"
{ yyval.i = Q_MOPRC; }
break;
case 61:
#line 226 "grammar.y"
{ yyval.rblk = gen_broadcast(yyvsp[-1].i); }
break;
case 62:
#line 227 "grammar.y"
{ yyval.rblk = gen_multicast(yyvsp[-1].i); }
break;
case 63:
#line 228 "grammar.y"
{ yyval.rblk = gen_less(yyvsp[0].i); }
break;
case 64:
#line 229 "grammar.y"
{ yyval.rblk = gen_greater(yyvsp[0].i); }
break;
case 65:
#line 230 "grammar.y"
{ yyval.rblk = gen_byteop(yyvsp[-1].i, yyvsp[-2].i, yyvsp[0].i); }
break;
case 66:
#line 231 "grammar.y"
{ yyval.rblk = gen_inbound(0); }
break;
case 67:
#line 232 "grammar.y"
{ yyval.rblk = gen_inbound(1); }
break;
case 68:
#line 234 "grammar.y"
{ yyval.i = BPF_JGT; }
break;
case 69:
#line 235 "grammar.y"
{ yyval.i = BPF_JGE; }
break;
case 70:
#line 236 "grammar.y"
{ yyval.i = BPF_JEQ; }
break;
case 71:
#line 238 "grammar.y"
{ yyval.i = BPF_JGT; }
break;
case 72:
#line 239 "grammar.y"
{ yyval.i = BPF_JGE; }
break;
case 73:
#line 240 "grammar.y"
{ yyval.i = BPF_JEQ; }
break;
case 74:
#line 242 "grammar.y"
{ yyval.a = gen_loadi(yyvsp[0].i); }
break;
case 76:
#line 245 "grammar.y"
{ yyval.a = gen_load(yyvsp[-3].i, yyvsp[-1].a, 1); }
break;
case 77:
#line 246 "grammar.y"
{ yyval.a = gen_load(yyvsp[-5].i, yyvsp[-3].a, yyvsp[-1].i); }
break;
case 78:
#line 247 "grammar.y"
{ yyval.a = gen_arth(BPF_ADD, yyvsp[-2].a, yyvsp[0].a); }
break;
case 79:
#line 248 "grammar.y"
{ yyval.a = gen_arth(BPF_SUB, yyvsp[-2].a, yyvsp[0].a); }
break;
case 80:
#line 249 "grammar.y"
{ yyval.a = gen_arth(BPF_MUL, yyvsp[-2].a, yyvsp[0].a); }
break;
case 81:
#line 250 "grammar.y"
{ yyval.a = gen_arth(BPF_DIV, yyvsp[-2].a, yyvsp[0].a); }
break;
case 82:
#line 251 "grammar.y"
{ yyval.a = gen_arth(BPF_AND, yyvsp[-2].a, yyvsp[0].a); }
break;
case 83:
#line 252 "grammar.y"
{ yyval.a = gen_arth(BPF_OR, yyvsp[-2].a, yyvsp[0].a); }
break;
case 84:
#line 253 "grammar.y"
{ yyval.a = gen_arth(BPF_LSH, yyvsp[-2].a, yyvsp[0].a); }
break;
case 85:
#line 254 "grammar.y"
{ yyval.a = gen_arth(BPF_RSH, yyvsp[-2].a, yyvsp[0].a); }
break;
case 86:
#line 255 "grammar.y"
{ yyval.a = gen_neg(yyvsp[0].a); }
break;
case 87:
#line 256 "grammar.y"
{ yyval.a = yyvsp[-1].a; }
break;
case 88:
#line 257 "grammar.y"
{ yyval.a = gen_loadlen(); }
break;
case 89:
#line 259 "grammar.y"
{ yyval.i = '&'; }
break;
case 90:
#line 260 "grammar.y"
{ yyval.i = '|'; }
break;
case 91:
#line 261 "grammar.y"
{ yyval.i = '<'; }
break;
case 92:
#line 262 "grammar.y"
{ yyval.i = '>'; }
break;
case 93:
#line 263 "grammar.y"
{ yyval.i = '='; }
break;
case 95:
#line 266 "grammar.y"
{ yyval.i = yyvsp[-1].i; }
break;
#line 1021 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = YYLEX) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
