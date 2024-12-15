%{
/*
 # $Id: gram.y,v 1.7 1998/04/10 10:27:18 fbm Exp fbm $
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
#include <assert.h>
#include <time.h>

#include "misc.h"
#include "morpher.h"

extern int		yyparse();
extern int		yylex();
extern void		yyerror(const char *msg);
#if defined(WIN32) || defined(GNU_WIN32)
extern char		*yytext;
#else
extern char		yytext[];
#endif
extern int		yyleng;

static morph_set	*newmorphset;
static morph_t		*newmorph;
static char		*newtoken;

static char		buff[1024];

%}

%token GRAMMAR TARGET TEXT_OFFSET DATA_OFFSET RODATA_OFFSET
%token TOKEN STRING NUM
%token INIT SEGMENT MSG GRADE BASE OFFSET OLD NEW CONTEXT
%token TEXT DATA RODATA
%token PRINT ASSERT PTR ADDR EQ NE LT LE GT GE

%union {
	enum verb_t	vt;
	enum op_t	ot;
	unsigned long	ul;
	morph_t		*pmorph;
	vec_t		*pvec;
	exp_t		*pexp;
}

%type <vt> verb
%type <ot> attr_name bool_op
%type <ul> seg_id num
%type <pmorph> morph
%type <pvec> vector
%type <pexp> exp

%%

start:		headers opt_morphs
		{
			morph_t	*p;

			p = find_morph(newmorphset, "_main");
			if (p)
				newmorphset->main = p->base + p->offset;

			/* if (newmorphset->main == ADDR_UNKN) {
				yyerror("No _main definition");
			} */
		}
		| error
		{
			yyerror("Parse error");
		};

headers:	init '{' grammar header_attrs '}'
		{
#if 0
			int	i;

			/* check for missing attributes in header */
			for (i = 0; i < NSEGS; i++) {
				if (newmorphset->offsets[i] == 0) {
					sprintf(buff, "Init section: No offset for %s segment.",
						seg_name(i));
					yyerror(buff);
				}
			}
#endif
		};

init:		INIT
		{
			newmorphset = f_calloc(1, sizeof(morph_set));
		};

grammar:	GRAMMAR num '.' num
		{
			newmorphset->main = ADDR_UNKN;
			newmorphset->nmorphs = 0;
			newmorphset->grammar_major = $2;
			newmorphset->grammar_minor = $4;

			if (newmorphset->grammar_major > GRAMMAR_MAJOR ||
				(newmorphset->grammar_major == GRAMMAR_MAJOR &&
					newmorphset->grammar_minor > GRAMMAR_MINOR))
				yyerror("Unsupported grammar version.");
		};

header_attrs:	header_attrs header_attr
		| header_attr
		;

header_attr:	TARGET STRING
		{
			if (newmorphset->target != (char *)0)
				free(newmorphset->target);
			newmorphset->target = f_strndup(yytext+1, yyleng-2);
		}
		| TEXT_OFFSET NUM
		{
			newmorphset->offsets[SEG_TEXT] = (long) yylval.ul;
		}
		| DATA_OFFSET NUM
		{
			newmorphset->offsets[SEG_DATA] = (long) yylval.ul;
		}
		| RODATA_OFFSET NUM
		{
			newmorphset->offsets[SEG_RODATA] = (long) yylval.ul;
		};

opt_morphs:	morphs
		| /* empty */
		;

morphs: 	morphs morph
		{
			int n = ++(newmorphset->morphs_sz);
			newmorphset->morphs =
				(morph_t **) f_realloc((void *) newmorphset->morphs,
				(n * sizeof(morph_t *)));
			newmorphset->morphs[n-1] = $2;
		}
		| morph
		{
			newmorphset->morphs_sz = 1;
			newmorphset->morphs = (morph_t **) f_malloc(sizeof(morph_t *));
			newmorphset->morphs[0] = $1;
		};

morph:		morph_name '{' morph_attrs '}'
		{
			/* check for missing attributes in morph definition */
			if (newmorph->seg == SEG_UNKN) {
				sprintf(buff, "Morph %s: No segment attribute", newmorph->name);
				yyerror(buff);
			}
			if (newmorph->base == ADDR_UNKN) {
				sprintf(buff, "Morph %s: No base address", newmorph->name);
				yyerror(buff);
			}
			if (newmorph->oldv.len <= 0) {
				sprintf(buff, "Morph %s: No old value", newmorph->name);
				yyerror(buff);
			}
			if (!newmorph->is_context && (newmorph->oldv.len != newmorph->newv.len)) {
				sprintf(buff, "Morph %s: Old length != new length", newmorph->name);
				yyerror(buff);
			}
			if (!newmorph->is_context)
				++(newmorphset->nmorphs);
			$$ = newmorph;
		};

morph_name:	TOKEN
		{
			newmorph = (morph_t *) f_calloc(1, sizeof(morph_t));
			newmorph->name = f_strndup(yytext, yyleng);
			newmorph->seg = SEG_UNKN;
			newmorph->base = ADDR_UNKN;
			newmorph->offset = 0;
			newmorph->grade = DEFAULT_GRADE;
		};

morph_attrs:	morph_attrs morph_attr
		| morph_attr
		;

morph_attr:	SEGMENT seg_id
		{
			newmorph->seg = $2;
		}
		| MSG STRING
		{
			if (newmorph->msg != (char *)0)
				free(newmorph->msg);
			newmorph->msg = f_strndup(yytext+1, yyleng-2);
		}
		| GRADE NUM
		{
			newmorph->grade = yylval.ul;
		}
		| BASE NUM
		{
			newmorph->base = yylval.ul;
		}
		| OFFSET NUM
		{
			newmorph->offset = (long) yylval.ul;
		}
		| OLD vector
		{
			if (newmorph->oldv.len != 0)
				free(newmorph->oldv.data);
			newmorph->oldv = *($2);
			free($2);
		}
		| NEW vector
		{
			if (newmorph->newv.len != 0)
				free(newmorph->newv.data);
			newmorph->newv = *($2);
			free($2);
		}
		| CONTEXT vector
		{
			if (newmorph->oldv.len != 0)
				free(newmorph->oldv.data);
			newmorph->oldv = *($2);
			newmorph->is_context = 1;
			free($2);
		}
		| verb exp
		{
			cmd_t	*cp, *p;

			cp = (cmd_t *) f_calloc(1, sizeof(cmd_t));
			cp->verb = $1;
			cp->exp = $2;
			if (newmorph->cmds == (cmd_t *)0)
				newmorph->cmds = cp;
			else {
				for (p = newmorph->cmds; p->next;)
					p = p->next;
				p->next = cp;
			}
		};

verb:		PRINT
		{
			$$ = print;
		}
		| ASSERT
		{
			$$ = assert;
		};

seg_id:		TEXT
		{
			$$ = SEG_TEXT;
		}
		| DATA
		{
			$$ = SEG_DATA;
		}
		| RODATA
		{
			$$ = SEG_RODATA;
		};

vector:		vector NUM
		{
			int n = ++(($1)->len);
			($1)->data = (unsigned char *) f_realloc((void *) (($1)->data),
							(n * sizeof(unsigned char)));
			($1)->data[n-1] = (unsigned char) yylval.ul;
			$$ = $1;
		}
		| NUM
		{
			$$ = f_malloc(sizeof(vec_t));
			($$)->format = VEC_NUM_FMT;
			($$)->len = 1;
			($$)->data = (unsigned char *) f_malloc(sizeof(unsigned char));
			($$)->data[0] = (unsigned char) yylval.ul;
		}
		| STRING
		{
			$$ = f_malloc(sizeof(vec_t));
			($$)->format = VEC_STR_FMT;
			($$)->data = (unsigned char *) f_strndup(yytext+1, yyleng-2);
			($$)->len = yyleng - 2;
			($$)->len = unescape((char *) ($$)->data, ($$)->len);
		};

exp:		num
		{
			$$ = (exp_t *) f_calloc(1, sizeof(exp_t));
			($$)->op = constant;
			($$)->nargs = 1;
			($$)->arg1.ul = $1;
		}
		| PTR '[' num ']'
		{
			$$ = (exp_t *) f_calloc(1, sizeof(exp_t));
			($$)->op = ptr;
			($$)->nargs = 1;
			($$)->arg1.ul = $3;
		}
		| PTR '[' num ',' num ']'
		{
			if ($3 >= $5) {
				sprintf(buff, "Morph %s: bad 'ptr' args", newmorph->name);
				yyerror(buff);
			}
			$$ = (exp_t *) f_calloc(1, sizeof(exp_t));
			($$)->op = ptr;
			($$)->nargs = 2;
			($$)->arg1.ul = $3;
			($$)->arg2.ul = $5;
		}
		| opt_token attr_name
		{
			$$ = (exp_t *) f_calloc(1, sizeof(exp_t));
			($$)->op = $2;
			($$)->nargs = 1;
			($$)->arg1.cp = newtoken;
		}
		|
		exp bool_op exp
		{
			$$ = (exp_t *) f_calloc(1, sizeof(exp_t));
			($$)->op = $2;
			($$)->nargs = 2;
			($$)->arg1.p = $1;
			($$)->arg2.p = $3;
		};

opt_token:	TOKEN
		{
			newtoken = f_strndup(yytext, yyleng);
		} '.'
		| /* empty */
		{
			newtoken = NULL;
		};

attr_name:	BASE
		{
			$$ = base;
		}
		| ADDR
		{
			$$ = addr;
		}
		| OFFSET
		{
			$$ = offset;
		};

bool_op:	EQ
		{
			$$ = eq;
		}
		| NE
		{
			$$ = ne;
		}
		| LT
		{
			$$ = lt;
		}
		| LE
		{
			$$ = le;
		}
		| GT
		{
			$$ = gt;
		}
		| GE
		{
			$$ = ge;
		};

num:		NUM
		{
			$$ = (long) yylval.ul;
		};

%%

morph_set *
parse(FILE* fp)
{
	lex_file_input(fp);
	yyparse();
	return newmorphset;
}

morph_t *
find_morph(morph_set *ms, char *name)
{
	int	i;
	morph_t	*p = (morph_t *)0;

	for (i = 0; i < ms->morphs_sz; i++) {
		p = ms->morphs[i];
		if (strcmp(p->name, name) == 0)
			break;
	}
	return p;
}

static void
dump_vec(FILE *fp, unsigned char *src, int len, int perline, int format)
{
	int	i;

	if (format == VEC_NUM_FMT) {
		if (len == 0)
			fputs("---------------", fp);		/* Highlight empty vectors. */
		for (i = 0; i < len; i++) {
			if (i > 0 && (i % perline) == 0) {
				fputs("\n\t\t\t", fp);
			}
			fprintf(fp, "0x%02x ", src[i]);
		}
		fputs("\n", fp);
	} else if (format == VEC_STR_FMT) {
		fputc('"', fp);
		for (; len-- > 0; src++) {
			if (*src == '\\')
				fputs("\\\\", fp);
			else if (isprint(*src))
				fputc(*src, fp);
			else
				fprintf(fp, "\\%03o", (unsigned int) *src);
		}
		fputs("\"\n", fp);
	}
}

void
dump_bool_op(FILE *fp, enum op_t op)
{
	if (op == eq)
		fputs("==", fp);
	else if (op == ne)
		fputs("!=", fp);
	else if (op == lt)
		fputs("<", fp);
	else if (op == le)
		fputs("<=", fp);
	else if (op == gt)
		fputs(">", fp);
	else if (op == ge)
		fputs(">=", fp);
}

static void
dump_exp(FILE *fp, exp_t *ep)
{
	if (ep->op == constant) {
		fprintf(fp, "%ld", ep->arg1.ul);
	}
	else if (ep->op == ptr) {
		fprintf(fp, "ptr[%ld", ep->arg1.ul);
		if (ep->nargs == 2)
			fprintf(fp, ",%ld", ep->arg2.ul);
		fputs("]", fp);
	}
	else if (ep->op == base || ep->op == addr || ep->op == offset) {
		if (ep->arg1.cp)
			fprintf(fp, "%s.", ep->arg1.cp);
		if (ep->op == base)
			fputs("base", fp);
		else if (ep->op == addr)
			fputs("addr", fp);
		else if (ep->op == offset)
			fputs("offset", fp);
	}
	else if (is_bool_op(ep->op)) {
		dump_exp(fp, ep->arg1.p);
		fputs(" ", fp);
		dump_bool_op(fp, ep->op);
		fputs(" ", fp);
		dump_exp(fp, ep->arg2.p);
	}
}

static void
dump_cmd(FILE *fp, cmd_t *cp)
{
	if (cp->verb == print)
		fputs("\tprint\t\t", fp);
	else if (cp->verb == assert)
		fputs("\tassert\t\t", fp);
	else {
		return;
	}
	dump_exp(fp, cp->exp);
	fputs("\n", fp);
}

void
dump_morph(FILE *fp, morph_set *ms, morph_t *mp, void (*eval_fn)(), void *vp)
{
	cmd_t	*cp;

	fprintf(fp, "\n%s {\n", mp->name);
	fprintf(fp, "\tsegment\t\t%s\n", seg_name(mp->seg));
	if (mp->msg)
		fprintf(fp, "\tmsg\t\t\"%s\"\n", mp->msg);
	if (mp->grade != DEFAULT_GRADE)
		fprintf(fp, "\tgrade\t\t%d\n", mp->grade);
	fprintf(fp, "\tbase\t\t0x%lx\n", mp->base);
	if (mp->seg == SEG_TEXT || mp->offset != 0)
		fprintf(fp, "\toffset\t\t0x%lx\n", mp->offset);
	if (mp->is_context) {
		fputs("\tcontext\t\t", fp);
		dump_vec(fp, mp->oldv.data, mp->oldv.len, 4, mp->oldv.format);
	} else {
		fputs("\told_value\t", fp);
		dump_vec(fp, mp->oldv.data, mp->oldv.len, 4, mp->oldv.format);
		fputs("\tnew_value\t", fp);
		dump_vec(fp, mp->newv.data, mp->newv.len, 4, mp->newv.format);
	}

	for (cp = mp->cmds; cp; cp = cp->next) {
		dump_cmd(fp, cp);
		if (eval_fn)
			(*eval_fn)(cp, ms, mp, fp, vp);
	}

	fputs("}\n", fp);
}

static void
dump_seg_offset(FILE *fp, char *label, long val)
{
	if (val < 0)
		fprintf(fp, "\t%s\t-0x%lx\n", label, -val);
	else
		fprintf(fp, "\t%s\t0x%lx\n", label, val);
}

void
dump_init(FILE *fp, morph_set *ms)
{
	time_t	tm;

	tm = time((time_t *)0);
	fprintf(fp, "\n; Morphs dump generated at %s", ctime(&tm));

	fputs("\ninit {\n", fp);
	fprintf(fp, "\tgrammar\t\t%d.%d\n", GRAMMAR_MAJOR, GRAMMAR_MINOR);
	fprintf(fp, "\ttarget\t\t\"%s\"\n", ms->target);
	dump_seg_offset(fp, "text_offset", ms->offsets[SEG_TEXT]);
	dump_seg_offset(fp, "data_offset", ms->offsets[SEG_DATA]);
	dump_seg_offset(fp, "rodata_offset", ms->offsets[SEG_RODATA]);
	/* fprintf(fp, "\tmain\t\t0x%x\n", ms->main); */
	fputs("}\n", fp);
}

void
dump_set(FILE *fp, morph_set *ms, void (*eval_fn)(), void *vp)
{
	int	i;

	dump_init(fp, ms);
	for (i = 0; i < ms->morphs_sz; i++)
		dump_morph(fp, ms, ms->morphs[i], eval_fn, vp);
}
