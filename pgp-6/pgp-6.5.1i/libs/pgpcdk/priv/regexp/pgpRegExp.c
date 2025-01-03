/*
 * regcomp and regexec -- regsub and regerror are elsewhere
 * @(#)regexp.c	1.3 of 18 April 87
 *
 *	Copyright (c) 1986 by University of Toronto.
 *	Written by Henry Spencer.  Not derived from licensed software.
 *
 *	Permission is granted to anyone to use this software for any
 *	purpose on any computer system, and to redistribute it freely,
 *	subject to the following restrictions:
 *
 *	1. The author is not responsible for the consequences of use of
 *		this software, no matter how awful, even if they arise
 *		from defects in it.
 *
 *	2. The origin of this software must not be misrepresented, either
 *		by explicit claim or by omission.
 *
 *	3. Altered versions must be plainly marked as such, and must not
 *		be misrepresented as being the original software.
 *
 * Beware that some of this code is subtly aware of the way operator
 * precedence is structured in regular expressions.  Serious changes in
 * regular-expression syntax might require a total rethink.
 */
#include <stdio.h>
#include <string.h>
#include "pgpRegExp.h"

#include "pgpDebug.h"
#include "pgpMem.h"

/*
 * The "internal use only" fields in regexp.h are present to pass info from
 * compile to execute that permits the execute phase to run lots faster on
 * simple cases.  They are:
 *
 * regstart	char that must begin a match; '\0' if none obvious
 * reganch	is the match anchored (at beginning-of-line only)?
 * regmust	string (pointer into program) that match must include, or NULL
 * regmlen	length of regmust string
 *
 * Regstart and reganch permit very fast decisions on suitable starting points
 * for a match, cutting down the work a lot.  Regmust permits fast rejection
 * of lines that cannot possibly match.  The regmust tests are costly enough
 * that regcomp() supplies a regmust only if the r.e. contains something
 * potentially expensive (at present, the only such thing detected is * or +
 * at the start of the r.e., which can involve a lot of backup).  Regmlen is
 * supplied because the test in regexec() needs it and regcomp() is computing
 * it anyway.
 */

#define NSUBEXP  10
struct regexp {
	char const *startp[NSUBEXP];
	char const *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char const *regmust;/* Internal use only. */
	int regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
};

/*
 * The first byte of the regexp internal "program" is actually this magic
 * number; the start node begins in the second byte.
 */
#define	MAGIC	((unsigned char)0234)


/*
 * Structure for regexp "program".  This is essentially a linear encoding
 * of a nondeterministic finite-state machine (aka syntax charts or
 * "railroad normal form" in parsing technology).  Each node is an opcode
 * plus a "next" pointer, possibly plus an operand.  "Next" pointers of
 * all nodes except BRANCH implement concatenation; a "next" pointer with
 * a BRANCH on both ends of it is connecting two alternatives.  (Here we
 * have one of the subtle syntax dependencies:  an individual BRANCH (as
 * opposed to a collection of them) is never concatenated with anything
 * because of operator precedence.)  The operand of some types of node is
 * a literal string; for others, it is a node leading into a sub-FSM.  In
 * particular, the operand of a BRANCH node is the first node of the branch.
 * (NB this is *not* a tree structure:  the tail of the branch connects
 * to the thing following the set of BRANCHes.)  The opcodes are:
 */

/* definition	number	opnd?	meaning */
#define	END	0	/* no	End of program. */
#define	BOL	1	/* no	Match "" at beginning of line. */
#define	EOL	2	/* no	Match "" at end of line. */
#define	ANY	3	/* no	Match any one character. */
#define	ANYOF	4	/* str	Match any character in this string. */
#define	ANYBUT	5	/* str	Match any character not in this string. */
#define	BRANCH	6	/* node	Match this alternative, or the next... */
#define	BACK	7	/* no	Match "", "next" ptr points backward. */
#define	EXACTLY	8	/* str	Match this string. */
#define	NOTHING	9	/* no	Match empty string. */
#define	STAR	10	/* node	Match this (simple) thing 0 or more times. */
#define	PLUS	11	/* node	Match this (simple) thing 1 or more times. */
#define	OPEN	20	/* no	Mark this point in input as start of #n. */
			/*	OPEN+1 is number 1, etc. */
#define	CLOSE	30	/* no	Analogous to OPEN. */

/*
 * Opcode notes:
 *
 * BRANCH	The set of branches constituting a single choice are hooked
 *		together with their "next" pointers, since precedence prevents
 *		anything being concatenated to any individual branch.  The
 *		"next" pointer of the last BRANCH in a choice points to the
 *		thing following the whole choice.  This is also where the
 *		final "next" pointer of each individual branch points; each
 *		branch starts with the operand node of a BRANCH node.
 *
 * BACK		Normal "next" pointers all implicitly point forward; BACK
 *		exists to make loop structures possible.
 *
 * STAR,PLUS	'?', and complex '*' and '+', are implemented as circular
 *		BRANCH structures using BACK.  Simple cases (one character
 *		per match) are implemented with STAR and PLUS for speed
 *		and to minimize recursive plunges.
 *
 * OPEN,CLOSE	...are numbered at compile time.
 */

/*
 * A node is one char of opcode followed by two chars of "next" pointer.
 * "Next" pointers are stored as two 8-bit pieces, high order first.  The
 * value is a positive offset from the opcode of the node containing it.
 * An operand, if any, simply follows the node.  (Note that much of the
 * code generation knows about this implicit relationship.)
 *
 * Using two bytes for the "next" pointer is vast overkill for most things,
 * but allows patterns to get big without disasters.
 */
#define	OP(p)	(*(p))
#define	NEXT(p)	(((*((p)+1)&0377)<<8) + (*((p)+2)&0377))
#define	OPERAND(p)	((p) + 3)

/*
 * See regmagic.h for one further detail of program structure.
 */


/*
 * Utility definitions.
 */
#define	UCHARAT(p)	((int)*(unsigned char *)(p))

#define	FAIL(m)	{ pgpDebugMsg(m); return(NULL); }
#define	ISMULT(c)	((c) == '*' || (c) == '+' || (c) == '?')
#define	META	"^$.[()|?+*\\"

/*
 * Flags to be passed up and down.
 */
#define	HASWIDTH	01	/* Known never to match null string. */
#define	SIMPLE		02	/* Simple enough to be STAR/PLUS operand. */
#define	SPSTART		04	/* Starts with * or +. */
#define	WORST		0	/* Worst case. */

/*
 * Global work variables for regcomp().
 */
static char regdummy;			/* Address for end of list */

typedef struct regcompState {
	char const *regparse;	/* Input-scan pointer. */
	int regnpar;			/* () count. */
	char *regcode;			/* Code-emit pointer; * &regdummy = don't. */
	long regsize;			/* Code size. */
} regcompState;

/*
 * Forward declarations for regcomp()'s friends.
 */
static char *reg(regcompState *rcs, int paren, int *flagp);
static char *regbranch(regcompState *rcs, int *flagp);
static char *regpiece(regcompState *rcs, int *flagp);
static char *regatom(regcompState *rcs, int *flagp);
static char *regnode(regcompState *rcs, char op);
static char *regnext(char const *p);
static void regc(regcompState *rcs, char b);
static void reginsert(regcompState *rcs, char op, char *opnd);
static void regtail(char *p, char const *val);
static void regoptail(char *p, char const *val);
static int strcspn_(const char *s1, const char *s2);


/*
 - regcomp - compile a regular expression into internal code
 *
 * We can't allocate space until we know how big the compiled form will be,
 * but we can't compile it (and thus know how big it is) until we've got a
 * place to put the code.  So we cheat:  we compile it twice, once with code
 * generation turned off and size counting turned on, and once "for real".
 * This also means that we don't allocate space until we are sure that the
 * thing really will compile successfully, and we never have to move the
 * code and thus invalidate pointers into it.  (Note that it has to be in
 * one piece because free() must be able to free it all.)
 *
 * Beware that the optimization-preparation code in here knows about some
 * of the structure of the compiled regexp.
 */
	PGPError
pgpRegComp(PGPContextRef context, char const *exp, regexp **pregexp)
{
	regexp *r;
	char const *scan;
	char const *longest;
	int len;
	int flags;
	regcompState s_rcs;
	regcompState *rcs = &s_rcs;

	PGPValidateContext( context );
	PGPValidatePtr( exp );
	PGPValidatePtr( pregexp );

	*pregexp = NULL;

	pgpClearMemory( &s_rcs, sizeof(s_rcs) );

	/* First pass: determine size, legality. */
	rcs->regparse = exp;
	rcs->regnpar = 1;
	rcs->regsize = 0L;
	rcs->regcode = &regdummy;
	regc(rcs, MAGIC);
	if (reg(rcs, 0, &flags) == NULL)
		return(kPGPError_OutOfMemory);

	/* Small enough for pointer-storage convention? */
	if (rcs->regsize >= 32767L)		/* Probably could be 65535L. */
		return(kPGPError_BadParams);

	/* Allocate space. */
	r = (regexp *)pgpContextMemAlloc(context,
							 sizeof(regexp) + (unsigned)rcs->regsize, 0);
	if (r == NULL)
		return kPGPError_OutOfMemory;

	/* Second pass: emit code. */
	rcs->regparse = exp;
	rcs->regnpar = 1;
	rcs->regcode = r->program;
	regc(rcs, MAGIC);
	if (reg(rcs, 0, &flags) == NULL)
		return(kPGPError_OutOfMemory);

	/* Dig out information for optimizations. */
	r->regstart = '\0';	/* Worst-case defaults. */
	r->reganch = 0;
	r->regmust = NULL;
	r->regmlen = 0;
	scan = r->program+1;			/* First BRANCH. */
	if (OP(regnext(scan)) == END) {	/* Only one top-level choice. */
		scan = OPERAND(scan);

		/* Starting-point info. */
		if (OP(scan) == EXACTLY)
			r->regstart = *OPERAND(scan);
		else if (OP(scan) == BOL)
			r->reganch++;

		/*
		 * If there's something expensive in the r.e., find the
		 * longest literal string that must appear and make it the
		 * regmust.  Resolve ties in favor of later strings, since
		 * the regstart check works with the beginning of the r.e.
		 * and avoiding duplication strengthens checking.  Not a
		 * strong reason, but sufficient in the absence of others.
		 */
		if (flags&SPSTART) {
			longest = NULL;
			len = 0;
			for (; scan != NULL; scan = regnext(scan))
				if (OP(scan) == EXACTLY
						&& strlen(OPERAND(scan)) >= (unsigned)len) {
					longest = OPERAND(scan);
					len = strlen(OPERAND(scan));
				}
			r->regmust = longest;
			r->regmlen = len;
		}
	}

	*pregexp = r;
	return(kPGPError_NoErr);
}

/*
 - reg - regular expression, i.e. main body or parenthesized thing
 *
 * Caller must absorb opening parenthesis.
 *
 * Combining parenthesis handling with the base level of regular expression
 * is a trifle forced, but the need to tie the tails of the branches to what
 * follows makes it hard to avoid.
 */
static char *
reg(regcompState *rcs, int paren, int *flagp)
{
	char *ret;
	char *br;
	char *ender;
	int parno = 0;
	int flags;

	*flagp = HASWIDTH;	/* Tentatively. */

	/* Make an OPEN node, if parenthesized. */
	if (paren) {
		if (rcs->regnpar >= NSUBEXP)
			FAIL("too many ()");
		parno = rcs->regnpar;
		rcs->regnpar++;
		ret = regnode(rcs, (char)(OPEN+parno));
	} else
		ret = NULL;

	/* Pick up the branches, linking them together. */
	br = regbranch(rcs, &flags);
	if (br == NULL)
		return(NULL);
	if (ret != NULL)
		regtail(ret, br);	/* OPEN -> first. */
	else
		ret = br;
	if (!(flags&HASWIDTH))
		*flagp &= ~HASWIDTH;
	*flagp |= flags&SPSTART;
	while (*rcs->regparse == '|') {
		rcs->regparse++;
		br = regbranch(rcs, &flags);
		if (br == NULL)
			return(NULL);
		regtail(ret, br);	/* BRANCH -> BRANCH. */
		if (!(flags&HASWIDTH))
			*flagp &= ~HASWIDTH;
		*flagp |= flags&SPSTART;
	}

	/* Make a closing node, and hook it on the end. */
	ender = regnode(rcs, (char)((paren) ? CLOSE+parno : END));	
	regtail(ret, ender);

	/* Hook the tails of the branches to the closing node. */
	for (br = ret; br != NULL; br = regnext(br))
		regoptail(br, ender);

	/* Check for proper termination. */
	if (paren && *rcs->regparse++ != ')') {
		FAIL("unmatched ()");
	} else if (!paren && *rcs->regparse != '\0') {
		if (*rcs->regparse == ')') {
			FAIL("unmatched ()");
		} else
			FAIL("junk on end");	/* "Can't happen". */
		/* NOTREACHED */
	}

	return(ret);
}

/*
 - regbranch - one alternative of an | operator
 *
 * Implements the concatenation operator.
 */
static char *
regbranch(regcompState *rcs, int *flagp)
{
	char *ret;
	char *chain;
	char *latest;
	int flags;

	*flagp = WORST;		/* Tentatively. */

	ret = regnode(rcs, BRANCH);
	chain = NULL;
	while (*rcs->regparse != '\0' && *rcs->regparse != '|'
		   && *rcs->regparse != ')') {
		latest = regpiece(rcs, &flags);
		if (latest == NULL)
			return(NULL);
		*flagp |= flags&HASWIDTH;
		if (chain == NULL)	/* First piece. */
			*flagp |= flags&SPSTART;
		else
			regtail(chain, latest);
		chain = latest;
	}
	if (chain == NULL)	/* Loop ran zero times. */
		(void) regnode(rcs, NOTHING);

	return(ret);
}

/*
 - regpiece - something followed by possible [*+?]
 *
 * Note that the branching code sequences used for ? and the general cases
 * of * and + are somewhat optimized:  they use the same NOTHING node as
 * both the endmarker for their branch list and the body of the last branch.
 * It might seem that this node could be dispensed with entirely, but the
 * endmarker role is not redundant.
 */
static char *
regpiece(regcompState *rcs, int *flagp)
{
	char *ret;
	char op;
	char *next;
	int flags;

	ret = regatom(rcs, &flags);
	if (ret == NULL)
		return(NULL);

	op = *rcs->regparse;
	if (!ISMULT(op)) {
		*flagp = flags;
		return(ret);
	}

	if (!(flags&HASWIDTH) && op != '?')
		FAIL("*+ operand could be empty");
	*flagp = (op != '+') ? (WORST|SPSTART) : (WORST|HASWIDTH);

	if (op == '*' && (flags&SIMPLE))
		reginsert(rcs, STAR, ret);
	else if (op == '*') {
		/* Emit x* as (x&|), where & means "self". */
		reginsert(rcs, BRANCH, ret);			/* Either x */
		regoptail(ret, regnode(rcs, BACK));		/* and loop */
		regoptail(ret, ret);				/* back */
		regtail(ret, regnode(rcs, BRANCH));		/* or */
		regtail(ret, regnode(rcs, NOTHING));	/* null. */
	} else if (op == '+' && (flags&SIMPLE))
		reginsert(rcs, PLUS, ret);
	else if (op == '+') {
		/* Emit x+ as x(&|), where & means "self". */
		next = regnode(rcs, BRANCH);			/* Either */
		regtail(ret, next);
		regtail(regnode(rcs, BACK), ret);		/* loop back */
		regtail(next, regnode(rcs, BRANCH));		/* or */
		regtail(ret, regnode(rcs, NOTHING));		/* null. */
	} else if (op == '?') {
		/* Emit x? as (x|) */
		reginsert(rcs, BRANCH, ret);			/* Either x */
		regtail(ret, regnode(rcs, BRANCH));		/* or */
		next = regnode(rcs, NOTHING);		/* null. */
		regtail(ret, next);
		regoptail(ret, next);
	}
	rcs->regparse++;
	if (ISMULT(*rcs->regparse))
		FAIL("nested *?+");

	return(ret);
}

/*
 - regatom - the lowest level
 *
 * Optimization:  gobbles an entire sequence of ordinary characters so that
 * it can turn them into a single node, which is smaller to store and
 * faster to run.  Backslashed characters are exceptions, each becoming a
 * separate node; the code is simpler that way and it's not worth fixing.
 */
static char *
regatom(regcompState *rcs, int *flagp)
{
	char *ret;
	int flags;

	*flagp = WORST;		/* Tentatively. */

	switch (*rcs->regparse++) {
	case '^':
		ret = regnode(rcs, BOL);
		break;
	case '$':
		ret = regnode(rcs, EOL);
		break;
	case '.':
		ret = regnode(rcs, ANY);
		*flagp |= HASWIDTH|SIMPLE;
		break;
	case '[': {
			int cclass;
			int classend;

			if (*rcs->regparse == '^') {	/* Complement of range. */
				ret = regnode(rcs, ANYBUT);
				rcs->regparse++;
			} else
				ret = regnode(rcs, ANYOF);
			if (*rcs->regparse == ']' || *rcs->regparse == '-')
				regc(rcs, *rcs->regparse++);
			while (*rcs->regparse != '\0' && *rcs->regparse != ']') {
				if (*rcs->regparse == '-') {
					rcs->regparse++;
					if (*rcs->regparse == ']' || *rcs->regparse == '\0')
						regc(rcs, '-');
					else {
						cclass = UCHARAT(rcs->regparse-2)+1;
						classend = UCHARAT(rcs->regparse);
						if (cclass > classend+1)
							FAIL("invalid [] range");
						for (; cclass <= classend; cclass++)
							regc(rcs, (char)cclass);
						rcs->regparse++;
					}
				} else
					regc(rcs, *rcs->regparse++);
			}
			regc(rcs, '\0');
			if (*rcs->regparse != ']')
				FAIL("unmatched []");
			rcs->regparse++;
			*flagp |= HASWIDTH|SIMPLE;
		}
		break;
	case '(':
		ret = reg(rcs, 1, &flags);
		if (ret == NULL)
			return(NULL);
		*flagp |= flags&(HASWIDTH|SPSTART);
		break;
	case '\0':
	case '|':
	case ')':
		FAIL("internal urp");	/* Supposed to be caught earlier. */
		break;
	case '?':
	case '+':
	case '*':
		FAIL("?+* follows nothing");
		break;
	case '\\':
		if (*rcs->regparse == '\0')
			FAIL("trailing \\");
		ret = regnode(rcs, EXACTLY);
		regc(rcs, *rcs->regparse++);
		regc(rcs, '\0');
		*flagp |= HASWIDTH|SIMPLE;
		break;
	default: {
			int len;
			char ender;

			rcs->regparse--;
			len = strcspn_(rcs->regparse, META);
			if (len <= 0)
				FAIL("internal disaster");
			ender = *(rcs->regparse+len);
			if (len > 1 && ISMULT(ender))
				len--;		/* Back off clear of ?+* operand. */
			*flagp |= HASWIDTH;
			if (len == 1)
				*flagp |= SIMPLE;
			ret = regnode(rcs, EXACTLY);
			while (len > 0) {
				regc(rcs, *rcs->regparse++);
				len--;
			}
			regc(rcs, '\0');
		}
		break;
	}

	return(ret);
}

/*
 - regnode - emit a node
 */
static char *			/* Location. */
regnode(regcompState *rcs, char op)
{
	char *ret;
	char *ptr;

	ret = rcs->regcode;
	if (ret == &regdummy) {
		rcs->regsize += 3;
		return(ret);
	}

	ptr = ret;
	*ptr++ = op;
	*ptr++ = '\0';		/* Null "next" pointer. */
	*ptr++ = '\0';
	rcs->regcode = ptr;

	return(ret);
}

/*
 - regc - emit (if appropriate) a byte of code
 */
static void
regc(regcompState *rcs, char b)
{
	if (rcs->regcode != &regdummy)
		*rcs->regcode++ = b;
	else
		rcs->regsize++;
}

/*
 - reginsert - insert an operator in front of already-emitted operand
 *
 * Means relocating the operand.
 */
static void
reginsert(regcompState *rcs, char op, char *opnd)
{
	char *src;
	char *dst;
	char *place;

	if (rcs->regcode == &regdummy) {
		rcs->regsize += 3;
		return;
	}

	src = rcs->regcode;
	rcs->regcode += 3;
	dst = rcs->regcode;
	while (src > opnd)
		*--dst = *--src;

	place = opnd;		/* Op node, where operand used to be. */
	*place++ = op;
	*place++ = '\0';
	*place++ = '\0';
}

/*
 - regtail - set the next-pointer at the end of a node chain
 */
static void
regtail(char *p, char const *val)
{
	char *scan;
	char *temp;
	int offset;

	if (p == &regdummy)
		return;

	/* Find last node. */
	scan = p;
	for (;;) {
		temp = regnext(scan);
		if (temp == NULL)
			break;
		scan = temp;
	}

	if (OP(scan) == BACK)
		offset = scan - val;
	else
		offset = val - scan;
	*(scan+1) = (offset>>8)&0377;
	*(scan+2) = offset&0377;
}

/*
 - regoptail - regtail on operand of first argument; nop if operandless
 */
static void
regoptail(char *p, char const *val)
{
	/* "Operandless" and "op != BRANCH" are synonymous in practice. */
	if (p == NULL || p == &regdummy || OP(p) != BRANCH)
		return;
	regtail(OPERAND(p), val);
}

/*
 * regexec and friends
 */

/*
 * Global work variables for regexec().
 */
typedef struct regexecState {
	char const *reginput;		/* String-input pointer. */
	char const *regbol;			/* Beginning of input, for ^ check. */
	char const **regstartp;		/* Pointer to startp array. */
	char const **regendp;		/* Ditto for endp. */
} regexecState;

#undef FAIL
#define	FAIL(m)	{ pgpDebugMsg(m); return(kPGPError_BadParams); }


/*
 * Forwards.
 */
static int regtry(regexecState *res, regexp *prog, char const *string);
static int regmatch(regexecState *res, char const *prog);
static int regrepeat(regexecState *res, char const *p);


#if REGEXP_DEBUG
int regnarrate = 0;
void regdump();
static char *regprop();
#endif

/*
 - regexec - match a regexp against a string
 */
int
pgpRegExec(regexp *prog, char const *string)
{
	char const *s;
	regexecState s_res;
	regexecState *res = &s_res;

	/* Be paranoid... */
	if (prog == NULL || string == NULL) {
		FAIL("NULL parameter");
	}

	/* Check validity of program. */
	if (UCHARAT(prog->program) != MAGIC) {
		FAIL("corrupted program");
		return(0);
	}

	pgpClearMemory( &s_res, sizeof(s_res) );

	/* If there is a "must appear" string, look for it. */
	if (prog->regmust != NULL) {
		s = string;
		while ((s = strchr(s, prog->regmust[0])) != NULL) {
			if (strncmp(s, prog->regmust, prog->regmlen) == 0)
				break;	/* Found it. */
			s++;
		}
		if (s == NULL)	/* Not present. */
			return(0);
	}

	/* Mark beginning of line for ^ . */
	res->regbol = string;

	/* Simplest case:  anchored match need be tried only once. */
	if (prog->reganch)
		return(regtry(res, prog, string));

	/* Messy cases:  unanchored match. */
	s = string;
	if (prog->regstart != '\0')
		/* We know what char it must start with. */
		while ((s = strchr(s, prog->regstart)) != NULL) {
			if (regtry(res, prog, s))
				return(1);
			s++;
		}
	else
		/* We don't -- general case. */
		do {
			if (regtry(res, prog, s))
				return(1);
		} while (*s++ != '\0');

	/* Failure. */
	return(0);
}

/*
 - regtry - try match at specific point
 */
static int			/* 0 failure, 1 success */
regtry(regexecState *res, regexp *prog, char const *string)
{
	int i;
	char const **sp;
	char const **ep;

	res->reginput = string;
	res->regstartp = prog->startp;
	res->regendp = prog->endp;

	sp = prog->startp;
	ep = prog->endp;
	for (i = NSUBEXP; i > 0; i--) {
		*sp++ = NULL;
		*ep++ = NULL;
	}
	if (regmatch(res, prog->program + 1)) {
		prog->startp[0] = string;
		prog->endp[0] = res->reginput;
		return(1);
	} else
		return(0);
}

/*
 - regmatch - main matching routine
 *
 * Conceptually the strategy is simple:  check to see whether the current
 * node matches, call self recursively to see whether the rest matches,
 * and then act accordingly.  In practice we make some effort to avoid
 * recursion, in particular by going through "ordinary" nodes (that don't
 * need to know whether the rest of the match failed) by a loop instead of
 * by recursion.
 */
static int			/* 0 failure, 1 success */
regmatch(regexecState *res, char const *prog)
{
	char const *scan;	/* Current node. */
	char const *next;		/* Next node. */

	scan = prog;
#if REGEXP_DEBUG
	if (scan != NULL && regnarrate)
		fprintf(stderr, "%s(\n", regprop(scan));
#endif
	while (scan != NULL) {
#if REGEXP_DEBUG
		if (regnarrate)
			fprintf(stderr, "%s...\n", regprop(scan));
#endif
		next = regnext(scan);

		switch (OP(scan)) {
		case BOL:
			if (res->reginput != res->regbol)
				return(0);
			break;
		case EOL:
			if (*res->reginput != '\0')
				return(0);
			break;
		case ANY:
			if (*res->reginput == '\0')
				return(0);
			res->reginput++;
			break;
		case EXACTLY: {
				int len;
				char const *opnd;

				opnd = OPERAND(scan);
				/* Inline the first character, for speed. */
				if (*opnd != *res->reginput)
					return(0);
				len = strlen(opnd);
				if (len > 1 && strncmp(opnd, res->reginput, len) != 0)
					return(0);
				res->reginput += len;
			}
			break;
		case ANYOF:
			if (*res->reginput == '\0' ||
				strchr(OPERAND(scan), *res->reginput) == NULL)
				return(0);
			res->reginput++;
			break;
		case ANYBUT:
			if (*res->reginput == '\0' ||
				strchr(OPERAND(scan), *res->reginput) != NULL)
				return(0);
			res->reginput++;
			break;
		case NOTHING:
			break;
		case BACK:
			break;
		case OPEN+1:
		case OPEN+2:
		case OPEN+3:
		case OPEN+4:
		case OPEN+5:
		case OPEN+6:
		case OPEN+7:
		case OPEN+8:
		case OPEN+9: {
				int no;
				char const *save;

				no = OP(scan) - OPEN;
				save = res->reginput;

				if (regmatch(res, next)) {
					/*
					 * Don't set startp if some later
					 * invocation of the same parentheses
					 * already has.
					 */
					if (res->regstartp[no] == NULL)
						res->regstartp[no] = save;
					return(1);
				} else
					return(0);
			}
			break;
		case CLOSE+1:
		case CLOSE+2:
		case CLOSE+3:
		case CLOSE+4:
		case CLOSE+5:
		case CLOSE+6:
		case CLOSE+7:
		case CLOSE+8:
		case CLOSE+9: {
				int no;
				char const *save;

				no = OP(scan) - CLOSE;
				save = res->reginput;

				if (regmatch(res, next)) {
					/*
					 * Don't set endp if some later
					 * invocation of the same parentheses
					 * already has.
					 */
					if (res->regendp[no] == NULL)
						res->regendp[no] = save;
					return(1);
				} else
					return(0);
			}
			break;
		case BRANCH: {
				char const *save;

				if (OP(next) != BRANCH)		/* No choice. */
					next = OPERAND(scan);	/* Avoid recursion. */
				else {
					do {
						save = res->reginput;
						if (regmatch(res, OPERAND(scan)))
							return(1);
						res->reginput = save;
						scan = regnext(scan);
					} while (scan != NULL && OP(scan) == BRANCH);
					return(0);
					/* NOTREACHED */
				}
			}
			break;
		case STAR:
		case PLUS: {
				char nextch;
				int no;
				char const *save;
				int min;

				/*
				 * Lookahead to avoid useless match attempts
				 * when we know what character comes next.
				 */
				nextch = '\0';
				if (OP(next) == EXACTLY)
					nextch = *OPERAND(next);
				min = (OP(scan) == STAR) ? 0 : 1;
				save = res->reginput;
				no = regrepeat(res, OPERAND(scan));
				while (no >= min) {
					/* If it could work, try it. */
					if (nextch == '\0' || *res->reginput == nextch)
						if (regmatch(res, next))
							return(1);
					/* Couldn't or didn't -- back up. */
					no--;
					res->reginput = save + no;
				}
				return(0);
			}
			break;
		case END:
			return(1);	/* Success! */
			break;
		default:
			FAIL("memory corruption");
			return(0);
			break;
		}

		scan = next;
	}

	/*
	 * We get here only if there's trouble -- normally "case END" is
	 * the terminating point.
	 */
	FAIL("corrupted pointers");
	return(0);
}

/*
 - regrepeat - repeatedly match something simple, report how many
 */
static int
regrepeat(regexecState *res, char const *p)
{
	int count = 0;
	char const *scan;
	char const *opnd;

	scan = res->reginput;
	opnd = OPERAND(p);
	switch (OP(p)) {
	case ANY:
		count = strlen(scan);
		scan += count;
		break;
	case EXACTLY:
		while (*opnd == *scan) {
			count++;
			scan++;
		}
		break;
	case ANYOF:
		while (*scan != '\0' && strchr(opnd, *scan) != NULL) {
			count++;
			scan++;
		}
		break;
	case ANYBUT:
		while (*scan != '\0' && strchr(opnd, *scan) == NULL) {
			count++;
			scan++;
		}
		break;
	default:		/* Oh dear.  Called inappropriately. */
		FAIL("internal foulup");
		count = 0;	/* Best compromise. */
		break;
	}
	res->reginput = scan;

	return(count);
}

/*
 - regnext - dig the "next" pointer out of a node
 */
static char *
regnext(char const *p)
{
	int offset;

	if (p == &regdummy)
		return(NULL);

	offset = NEXT(p);
	if (offset == 0)
		return(NULL);

	if (OP(p) == BACK)
		return(char *)(p-offset);
	else
		return(char *)(p+offset);
}

#if REGEXP_DEBUG

static char *regprop();

/*
 - regdump - dump a regexp onto stdout in vaguely comprehensible form
 */
void
regdump(regexp const *r)
{
	char *s;
	char op = EXACTLY;	/* Arbitrary non-END op. */
	char *next;
	extern char *strchr();


	s = r->program + 1;
	while (op != END) {	/* While that wasn't END last time... */
		op = OP(s);
		printf("%2d%s", s-r->program, regprop(s));	/* Where, what. */
		next = regnext(s);
		if (next == NULL)		/* Next ptr. */
			printf("(0)");
		else 
			printf("(%d)", (s-r->program)+(next-s));
		s += 3;
		if (op == ANYOF || op == ANYBUT || op == EXACTLY) {
			/* Literal string, where present. */
			while (*s != '\0') {
				putchar(*s);
				s++;
			}
			s++;
		}
		putchar('\n');
	}

	/* Header fields of interest. */
	if (r->regstart != '\0')
		printf("start `%c' ", r->regstart);
	if (r->reganch)
		printf("anchored ");
	if (r->regmust != NULL)
		printf("must have \"%s\"", r->regmust);
	printf("\n");
}

/*
 - regprop - printable representation of opcode
 */
static char *
regprop(op)
char *op;
{
	char *p;
	static char buf[50];

	(void) strcpy(buf, ":");

	switch (OP(op)) {
	case BOL:
		p = "BOL";
		break;
	case EOL:
		p = "EOL";
		break;
	case ANY:
		p = "ANY";
		break;
	case ANYOF:
		p = "ANYOF";
		break;
	case ANYBUT:
		p = "ANYBUT";
		break;
	case BRANCH:
		p = "BRANCH";
		break;
	case EXACTLY:
		p = "EXACTLY";
		break;
	case NOTHING:
		p = "NOTHING";
		break;
	case BACK:
		p = "BACK";
		break;
	case END:
		p = "END";
		break;
	case OPEN+1:
	case OPEN+2:
	case OPEN+3:
	case OPEN+4:
	case OPEN+5:
	case OPEN+6:
	case OPEN+7:
	case OPEN+8:
	case OPEN+9:
		sprintf(buf+strlen(buf), "OPEN%d", OP(op)-OPEN);
		p = NULL;
		break;
	case CLOSE+1:
	case CLOSE+2:
	case CLOSE+3:
	case CLOSE+4:
	case CLOSE+5:
	case CLOSE+6:
	case CLOSE+7:
	case CLOSE+8:
	case CLOSE+9:
		sprintf(buf+strlen(buf), "CLOSE%d", OP(op)-CLOSE);
		p = NULL;
		break;
	case STAR:
		p = "STAR";
		break;
	case PLUS:
		p = "PLUS";
		break;
	default:
		regerror("corrupted opcode");
		break;
	}
	if (p != NULL)
		(void) strcat(buf, p);
	return(buf);
}
#endif

/*
 * The following is provided for those people who do not have strcspn() in
 * their C libraries.  They should get off their butts and do something
 * about it; at least one public-domain implementation of those (highly
 * useful) string routines has been published on Usenet.
 */

/*
 * strcspn_ - find length of initial segment of s1 consisting entirely
 * of characters not from s2
 */

static int
strcspn_(const char *s1, const char *s2)
{
	char const *scan1;
	char const *scan2;
	int count;

	count = 0;
	for (scan1 = s1; *scan1 != '\0'; scan1++) {
		for (scan2 = s2; *scan2 != '\0';)	/* ++ moved down. */
			if (*scan1 == *scan2++)
				return(count);
		count++;
	}
	return(count);
}





/*__Editor_settings____

	Local Variables:
	tab-width: 4
	comment-column: 40
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
