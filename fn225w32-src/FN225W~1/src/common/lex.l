%{
/*
 # $Id: lex.l,v 1.6 1998/03/10 14:11:33 fbm Exp fbm $
 # Copyright (C) 1997-2000 Farrell McKay
 # All rights reserved.
 #
 # This file is part of the Fortify distribution, a toolkit for
 # upgrading the cryptographic strength of the Netscape web browsers.
 #
 # This toolkit is provided to the recipient under the
 # following terms and conditions:-
 #   1.  This copyright notice must not be removed or modified.
 #   2.  This toolkit may not be reproduced or included in any commercial
 #       media distribution, or commercial publication (for example CD-ROM,
 #       disk, book, magazine, journal) without first obtaining the author's
 #       express permission.
 #   3.  This toolkit, or any component of this toolkit, may not be
 #       used, resold, redeveloped, rewritten, enhanced or otherwise
 #       employed by a commercial organisation, or as part of a commercial
 #       venture, without first obtaining the author's express permission.
 #   4.  Subject to the above conditions being observed (1-3), this toolkit
 #       may be freely reproduced or redistributed.
 #   5.  To the extent permitted by applicable law, this software is
 #       provided "as-is", without warranty of any kind, including
 #       without limitation, the warrantees of merchantability,
 #       freedom from defect, non-infringement, and fitness for
 #       purpose.  In no event shall the author be liable for any
 #       direct, indirect or consequential damages however arising
 #       and however caused.
 #   6.  Subject to the above conditions being observed (1-5),
 #       this toolkit may be used at no cost to the recipient.
 #
 # Farrell McKay
 # Wayfarer Systems Pty Ltd		contact@fortify.net
 */

#include "includes.h"

#include "misc.h"
#include "morpher.h"

#if defined(OS2)
#include "y_tab.h"
#elif defined(macintosh)
#include "gram.tab.h"
#else
#include "y.tab.h"
#endif

#ifndef YYLMAX
#define YYLMAX 1024
#endif

extern YYSTYPE		yylval;

static int		linenum;

extern int		yylex();

void
lex_file_input(FILE *fp)
{
	linenum = 1;
	yyin = fp;
}

int
yywrap(void)
{
	return 1;
}

void
yyerror(const char *msg)
{
	fprintf(stderr, "%s (at or before line %d)\n", msg, linenum);
	exit(2);
}

/*
%p n       Number of positions                     2500 
%n n       Number of states                         500 
%a n       Number of transitions                   2000 
%e n       Number of parse tree nodes              1000 
%k n       Number of packed character classes     10000 
%o n       Size of the output array                3000
*/

%}

%a			 5000
%o			10000

D			[0-9]
H			[0-9a-f]
E			(\\.)
Z			[^ \f\t\n\.{}\[\],;"]

%%

#.*\n			{ linenum++; }
;.*\n			{ linenum++; }

init			{ return INIT; }
grammar			{ return GRAMMAR; }
target			{ return TARGET; }
text_offset		{ return TEXT_OFFSET; }
data_offset		{ return DATA_OFFSET; }
rodata_offset		{ return RODATA_OFFSET; }

segment			{ return SEGMENT; }
msg			{ return MSG; }
grade			{ return GRADE; }
base			{ return BASE; }
offset			{ return OFFSET; }
old_value		{ return OLD; }
new_value		{ return NEW; }
context			{ return CONTEXT; }

text			{ return TEXT; }
data			{ return DATA; }
rodata			{ return RODATA; }

print			{ return PRINT; }
assert			{ return ASSERT; }
ptr			{ return PTR; }
addr			{ return ADDR; }

==			{ return EQ; }
!=			{ return NE; }
\<			{ return LT; }
\<=			{ return LE; }
\>			{ return GT; }
\>=			{ return GE; }

\"(\\.|[^\\"\n])*\"	{ return STRING; }

[+-]?0x{H}+		{ yylval.ul = (unsigned long) strtol(yytext, (char **)0, 0); return NUM; }
[+-]?{D}+		{ yylval.ul = (unsigned long) strtol(yytext, (char **)0, 0); return NUM; }

[ \f\t]			;
\n                      { linenum++; }

({E}|{Z})+		{ return TOKEN; }

.			{ return yytext[0]; }

%%
