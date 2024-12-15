/*
 # $Id: morpher.h,v 1.5 1998/04/10 10:27:56 fbm Exp fbm $
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

#ifndef MORPHER_H
#define MORPHER_H

#include "index.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	ADDR_UNKN	(unsigned long)-1
#define GRAMMAR_MAJOR	1
#define GRAMMAR_MINOR	2

#define DEFAULT_GRADE	1

#define	VEC_NUM_FMT	0
#define	VEC_STR_FMT	1

#define ERR_OPEN	1
#define ERR_NOMATCH	2
#define ERR_LXCOMPR	3
#define ERR_ISSCRIPT	4

#if defined(WIN32) || defined(GNU_WIN32) || defined(OS2)
#define OPENFL(a)	((a)|O_BINARY)
#define PATH_DELIM	';'
#define PATH_SEP	'\\'
#define PROG_NAME	"fortify.exe"
#elif defined(macintosh)
#define OPENFL(a)	((a)|O_BINARY)
#define PATH_DELIM	';'
#define PATH_SEP	':'
#define PROG_NAME	"fortify"
#else
#define OPENFL(a)	(a)
#define PATH_DELIM	':'
#define PATH_SEP	'/'
#define PROG_NAME	"fortify"
#endif

#if !defined(OS2) && !defined(WIN32)
#define	min(a,b)	((a) < (b)? (a): (b))
#define	max(a,b)	((a) > (b)? (a): (b))
#endif

#define is_bool_op(a)	((a)==eq||(a)==ne||(a)==lt||(a)==le||(a)==gt||(a)==ge)

enum verb_t {
	print, assert
};

enum op_t {
	constant, ptr, base, addr, offset, eq, ne, lt, le, gt, ge
};

typedef struct {
	int		len;
	unsigned char	*data;
	int		format;
} vec_t;

typedef struct exp {
	enum op_t	op;
	int		nargs;
	union {
	  unsigned long ul;
	  struct exp	*p;
	  char		*cp;
	}		arg1, arg2;
} exp_t;

typedef struct cmd {
	struct cmd	*next;
	enum verb_t	verb;
	exp_t		*exp;
} cmd_t;

typedef struct {
	char		*name;
	int		seg;
	char		*msg;
	unsigned long	base;
	unsigned long	offset;
	unsigned long	filepos;
	int		grade;
	int		is_context;
	vec_t		oldv, newv;
	cmd_t		*cmds;
} morph_t;

typedef struct {
	int		grammar_major;
	int		grammar_minor;
	char		*target;
	unsigned long	main;
	long		offsets[NSEGS];
	int		nmorphs;		/* no. of morphs in morphs[] */
	int		morphs_sz;		/* len of morphs[] array */
	morph_t		**morphs;
} morph_set;

morph_set	*parse(FILE* fp);
void		lex_file_input(FILE* fp);
int		yywrap(void);
void		yyerror(const char *msg);
morph_t		*find_morph(morph_set *ms, char *name);
void		dump_bool_op(FILE *fp, enum op_t op);
void		dump_morph(FILE *fp, morph_set *ms, morph_t *mp, void (*eval_fn)(), void *vp);
void		dump_init(FILE *fp, morph_set *ms);
void		dump_set(FILE *fp, morph_set *ms, void (*eval_fn)(), void *vp);
int		morpher(const char *tgt, char *morph_file, tgt_info_t *tinf, int old_grade, int new_grade, char *action);

#ifdef __cplusplus
}
#endif

#endif
