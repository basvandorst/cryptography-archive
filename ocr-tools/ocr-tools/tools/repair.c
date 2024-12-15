/*
 * repair.c -- Program which reconstructs scanned source, locates errors,
 *			   and tries to fix most of them automatically.  If it
 *             can't, it drops you into an editor on the appropriate
 *             line for manual correction.
 *
 * Given a file "foo", this appends corrected output to "foo.out"
 * and copies remaining uncorrected input in "foo.in".  If "foo.in"
 * exists initially, "foo" is ignored and only "foo.in" is processed.
 * Thus, re-running it repeatedly, possibly with other correction
 * techniques in between, will result in correct output in "foo.out"
 * and an empty "foo.in" file.
 *
 * This can automatically invoke an editor for you on the .in file
 * and re-run itself.  The editor is chosen in the first available way:
 * - The -e command-line argument takes a printf() format string to
 *   format the editor invocation command line with the line number and
 *   filename.  E.g. "emacs +%u %s".  %u and %s must appear, in that order.
 * - Failing that, the default is "$VISUAL +%u %s"
 * - Failing that, the default is "$EDITOR +%u %s"
 * - Failing that, the program prints the error location and exits.
 *   Specifying -e- forces this behaviour.
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.
 *
 * Designed by Colin Plumb, Mark H. Weaver, and Philip R. Zimmermann
 * Written by Colin Plumb
 *
 * $Id: repair.c,v 1.37 1997/11/14 08:39:40 mhw Exp $
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>

#include "util.h"
#include "heap.h"
#include "mempool.h"
#include "subst.h"

/*
 * The internal form of a substitution.  These are stored on
 * lists indexed by the first character of the input substitution.
 */
typedef struct Substitution {
	struct Substitution *next;
	char const *input, *output;
	size_t inlen, outlen;
	HeapCost cost, cost2;
	FilterFunc *filter;
	unsigned int index;	/* Consecutive serial numbers */
} Substitution;

struct Substitution const substNull = { NULL, "", "", 0, 0, 0, 0, 0 };

/*
 * This might get increased later to support multiple classes of
 * substitutions, for different contexts.  Currently, only one
 * is used.
 */
#define SUBST_CLASSES 1

/* List of substitutions, indexed by first character, plus a catch-all */
Substitution *substitutions[SUBST_CLASSES][0x101];

/*
 * The pool of Substitution structures.  Remains alive for the entire
 * execution of the program.
 */
static MemPool substPool;
static Substitution *substFree;
static unsigned int substCount = 1;	/* Preallcoate 0 to substNull */
static unsigned int substFirstDynamic;
#define SubstIsDynamic(s) ((s)->index >= substFirstDynamic)
/* Adjust the substitution based on noccurrences this page */
#define SubstAdjust(s,n) ((s)->cost = (s)->cost2)
/* Is this a nasty-line substitution? */
#define SubstIsNasty(s) ((s)->cost2 == COST_INFINITY)

/* Every possible single-character string */
static char substChars[512];
#define SubstString(c) (substChars+2*((c)&255))

/* Set the list of substitutions to empty */
static void
SubstInit(void)
{
	unsigned int i, j;

	memPoolInit(&substPool);
	substFree = 0;
	substCount = 1;	/* Number zero is reserved for uncounted substitutions */
	for (i = 0; i < elemsof(substitutions); i++)
		for (j = 0; j < elemsof(*substitutions); j++)
			substitutions[i][j] = NULL;

	for (i = 0; i < 256; i++) {
		substChars[2*i] = (char)i;
		substChars[2*i+1] = 0;
	}
}

/*
 * For dynamically allocated substitutions, we maintain a free list.
 * Each substitution has a unique serial number.  These are retained
 * if a substitution goes on the free list, to keep substCount from
 * ratcheting upwards indefinitely while still guaranteeing uniqueness.
 */
static Substitution *
SubstAlloc(void)
{
	struct Substitution *subst = substFree;

	if (subst) {
		substFree = subst->next;
	} else {
		subst = memPoolNew(&substPool, Substitution);
		subst->index = substCount++;
	}
	return subst;
}

static void
SubstFree(Substitution *subst)
{
	subst->next = substFree;
	substFree = subst;
}

static Substitution *
MakeSubst(char const *input, char const *output, HeapCost cost, HeapCost cost2,
	FilterFunc *filter, int class)
{
	struct Substitution *subst, **head;

	subst = SubstAlloc();
	subst->input = input;
	subst->output = output;
	subst->inlen = strlen(input);
	subst->outlen = strlen(output);
	subst->cost = cost;
	subst->cost2 = cost2;
	subst->filter = filter;

	/*
	 * Ignore certain substitutions when printing stats.
	 * Identity substitutions, and the tab/space tweaking.
	 */
	if (strcmp(input, output) == 0 || strcmp(input, TAB_STRING) == 0 ||
		(input[0] == ' ' && input[1] == 0 && output[0] == 0)) {
			if (subst->index == substCount-1)
				substCount--;
			subst->index = 0;	/* Evil hack */
	}

	head = &substitutions[class][input[class] & 255];
	subst->next = *head;
	*head = subst;
	return subst;
}

/*
 * For each entry in the raw array, turn { "abc", "def", 5" }
 * into cost-5 mappings of "a"->"d", "b"->"e" and "c"->"f".
 * If the output string is NULL, the characters are deleted.
 * An input string of NULL is the end of table delimiter.
 */
static void
SubstSingle(struct RawSubst const *raw, int class)
{
	char const *input, *output;
	int i, o;

	while (raw->input) {
		input = raw->input;
		output = raw->output;
		assert(!output || strlen(input) == strlen(output));

		while (*input) {
			i = *input++;
			o = output ? *output++ : 0;
			(void)MakeSubst(SubstString(i), SubstString(o),
							raw->cost, raw->cost2, raw->filter, class);
		}
		raw++;
	}
}

/*
 * For each entry in the raw array, turn { "abc", "def", 5" }
 * into a cost-5 mappings of "abc"->"def".
 * An input string of NULL is the end of table delimiter.
 */
static void
SubstMultiple(struct RawSubst const *raw, int class)
{
	while (raw->input) {
		(void)MakeSubst(raw->input, raw->output, raw->cost, raw->cost2,
					    raw->filter, class);
		raw++;
	}
}

/* Build the substitutions table */
static void
SubstBuild(void)
{
	SubstInit();
	SubstSingle(substSingles, 0);
	SubstMultiple(substMultiples, 0);
	substFirstDynamic = substCount;
}

/*
 * See if the desired substitution already exists
 */
static Substitution const *
SubstSearch(char const *in, size_t inlen, char const *out, size_t outlen,
	int class)
{
	Substitution *subst = substitutions[class][in[0] & 255];

	for (; subst; subst = subst->next) {
		if (subst->inlen == inlen && subst->outlen == outlen &&
			memcmp(subst->input, in, inlen) == 0 &&
			memcmp(subst->output, out, outlen) == 0)
				return subst;	/* Already exists */
	}
	return NULL;
}


/*
 * Create a new dynamic substitution.  First search to make
 * sure it doesn't already esist.
 */
static Substitution const *
SubstDynamic(char const *in, char const *out, int class)
{
	Substitution const *subst;

	subst = SubstSearch(in, strlen(in), out, strlen(out), class);
	return subst ? subst : MakeSubst(in, out, COST_INFINITY,
									 DYNAMIC_COST_LEARNED, NULL, class);
}

/*
 * Search for the substitution, allocating one if not found.
 * the input string is not null-terminated and needs to be copied to
 * an allocated buffer.  The output string can just be pointer-copied.
 */
static Substitution const *
SubstNasty(char const *in, size_t inlen, char const *out, int class)
{
	Substitution const *subst;
	char *string;

	if ((subst = SubstSearch(in, inlen, out, strlen(out), class)) != NULL)
		return subst;

	if (!(string = malloc(inlen+1))) {
		fputs("Out of memory!\n", stderr);
		exit(1);
	}
	memcpy(string, in, inlen);
	string[inlen] = 0;
	return MakeSubst(string, out, COST_INFINITY, COST_INFINITY, NULL, class);
}

/*
 * The state of the parser.
 * Note that this is updated when a ParseNode is *removed* from the heap;
 * ParseNodes that are in the heap have ParseStates that reflect the
 * state before the substitution has been parsed; this is a copy of the
 * parents' state, which is after the parsing.
 */
typedef struct ParseState {
	CRC page_crc;			/* Computed per-page CRC */
	word16 flags;			/* Flags; see below */
	unsigned char pos;		/* Position on the line */
} ParseState;	/* 7 bytes, rounded to 8 */

/* Flags values */
#define PS_MASK_PAGENUM	 0xC000 /* Digits in header page number (1..3) */
#define PS_SHIFT_PAGENUM	 14	/* Shift for the above */
#define PS_FLAG_EOL			512	/* Expect \n next */
#define PS_FLAG_SPACE		256	/* Was last char a space? */
#define PS_FLAG_TAB			128	/* Tabbing over a column */
#define PS_FLAG_INHEADER	 64	/* Current line is a header */
#define PS_FLAG_PASTHEADER	 32	/* A previous line was a header */
#define PS_FLAG_BINWS		 16	/* In whitespace after binary data */
#define PS_FLAG_BINEND		  8	/* End of binary data */
#define PS_FLAG_DYNAMIC		  4	/* Have used ECC this line */
#define PS_MASK_FORMAT	 	  3	/* The encoding format (max of 3, for now) */
#define PS_SHIFT_FORMAT		  0	/* Shift for the above */

/* Have we started on a second page?  Used to force flushing of the first. */
#define InSecondHeader(ps) \
	((~(ps)->flags & (PS_FLAG_INHEADER | PS_FLAG_PASTHEADER)) == 0)

#define PageNumDigits(pn) (((pn)->ps.flags & PS_MASK_PAGENUM) >> PS_SHIFT_PAGENUM)
#define PageNumDigitsIncrement(pn) ((pn)->ps.flags += 1<<PS_SHIFT_PAGENUM)

EncodeFormat const *registeredFormats[4];

/* Returns a small integer index */
static int
registerFormat(EncodeFormat const *format)
{
	int i;
	for (i = 0; i < (int)elemsof(registeredFormats); i++) {
		if (registeredFormats[i] == format)
			return i;
		if (!registeredFormats[i]) {
			registeredFormats[i] = format;
			return i;
		}
	}
	fputs("Registered formats table overflow!\n", stderr);
	exit(1);
}

#define psFormat(ps) registeredFormats[((ps)->flags & PS_MASK_FORMAT)>>PS_SHIFT_FORMAT]
#define pnFormat(pn) psFormat(&(pn)->ps)
#define psSetFormat(ps, i) \
	((ps)->flags = ((ps)->flags & ~PS_MASK_FORMAT) | i << PS_SHIFT_FORMAT)

typedef struct ParseNode {
	HeapCost cost;
	unsigned int refcnt;
	struct ParseNode *parent;
	char const *input;
	struct Substitution const *subst;
	struct ParseState ps;
} ParseNode;	/* 32 bytes */

/* A handle for walking backwards through the output stream */
typedef struct OutputHandle {
	ParseNode const *node;
	char const *output;
	unsigned int pos;
} OutputHandle;

/* Initialize the handle to point to a node (optionally, a position therein) */
static void
OutputInit(OutputHandle *oh, ParseNode const *node, char const *p)
{
		oh->node = node;
		oh->output = p ? p : node->subst->output + node->subst->outlen;
		oh->pos = 0;
}

/* Get the *previous* byte */
static int
OutputGetPrev(OutputHandle *oh)
{
	if (!oh->node)
		return -1;
	for (;;) {
		if (oh->output != oh->node->subst->output) {
			oh->pos++;
			return *--oh->output & 255;
		}
		oh->node = oh->node->parent;
		if (!oh->node)
			break;
		oh->output = oh->node->subst->output + oh->node->subst->outlen;
	}
	return -1;
}

/* Return the character just before the node - trivial handy wrapper */
static int
OutputPrevChar(ParseNode const *node)
{
	OutputHandle oh;

	OutputInit(&oh, node, NULL);
	return OutputGetPrev(&oh);
}

/*
 * Unget the last retrieved character (and return it), or
 * -1 if that is impossible.  At least one character is
 * always ungettable, but after that you're on your own.
 */
static int
OutputUnget(OutputHandle *oh)
{
	if (oh->node && *oh->output) {
		oh->pos--;
		return *oh->output++ & 255;
	}
	return -1;
}

/* The position is useful for comparing two OutputHandles. */
#define OutputPos(oh) ((oh)->pos)

/*
 * Fill backwards from bufend until you hit the given char.
 * Use -1 to get the whole buffer.
 */
static char *
OutputGetUntil(OutputHandle oh, char *bufend, int end)
{
	int c;

	while ((c = OutputGetPrev(&oh)) != -1 && c != end)
		*--bufend = (char)c;
	return bufend;
}

/*
 * The per-page structure.  This is actually global, but describes
 * the values kept for each page processed.
 */
typedef struct PerPage {
	CRC page_check;
	char const *maxpos, *minpos;
	unsigned int tabsize;	/* Zero means this is a binary page */
	unsigned int lines;
	unsigned int retries;	/* How many retires since last progress? */
	unsigned int max_retries;	/* Maximum number of retries needed. */
} PerPage;

PerPage perpage;	/* The global */

static void
PerPageInit(char const *buf)
{
	perpage.maxpos = perpage.minpos = buf;
	perpage.page_check = 0;
	perpage.tabsize = 4;	/* The default */
	perpage.lines = perpage.retries = perpage.max_retries = 0;
}

/*
 * Is the tab substitution being looked at acceptable?
 * It is if the length needed to make the tab width come out
 * right, it is.  Otherwise, it's junk.
 */
HeapCost
TabFilter(struct ParseNode *parent, char const *limit,
	struct Substitution const *subst)
{
	int c, tabpos;
	OutputHandle oh;

	(void)limit;
	if (!perpage.tabsize)
		return COST_INFINITY;	/* No interest */

	/* How wide should the tab be? */
	tabpos = (int)((parent->ps.pos-PREFIX_LENGTH) % perpage.tabsize);
	if ((int)subst->outlen != (int)perpage.tabsize - tabpos)
		return COST_INFINITY;
	/* The right number - cost if likely, cost2 if unlikely */
	if (subst->cost == subst->cost2)
		return subst->cost;
	OutputInit(&oh, parent, NULL);
	do {
		c = OutputGetPrev(&oh);
	} while (c == ' ');
	return (c == TAB_CHAR) ? subst->cost : subst->cost2;
}

/*
 * Return cost if near blanks (including end-of-line), cost2 if not, and
 * the average of there is a blank on one side.  There are additional
 * versions for upper- and lower-case.  _ is considered upper-case,
 * as it's oftne used in acro identifiers.
 */
HeapCost
FilterNearBlanks(struct ParseNode *parent, char const *limit,
	struct Substitution const *subst)
{
	int c = OutputPrevChar(parent), score = (isspace(c) != 0);
	char const *p = parent->input + parent->subst->inlen;

	score += p == limit || isspace((unsigned char)*p) != 0;
	return (subst->cost*score + subst->cost2*(2-score))/2;
}

HeapCost
FilterNearUpper(struct ParseNode *parent, char const *limit,
	struct Substitution const *subst)
{
	int c = OutputPrevChar(parent), score = (isupper(c) != 0 || c == '_');
	char const *p = parent->input + subst->inlen;

	score += p != limit && (isupper((unsigned char)*p) != 0 || *p == '_');
	return (subst->cost*score + subst->cost2*(2-score))/2;
}

HeapCost
FilterNearXDigit(struct ParseNode *parent, char const *limit,
	struct Substitution const *subst)
{
	int c = OutputPrevChar(parent), score = (isxdigit(c) != 0);
	char const *p = parent->input + subst->inlen;

	score += p != limit && (isxdigit((unsigned char)*p) != 0);
	return (subst->cost*score + subst->cost2*(2-score))/2;
}

HeapCost
FilterNearLower(struct ParseNode *parent, char const *limit,
	struct Substitution const *subst)
{
	int c = OutputPrevChar(parent), score = (islower(c) != 0);
	char const *p = parent->input + subst->inlen;

	score += p != limit && (islower((unsigned char)*p) != 0);
	return (subst->cost*score + subst->cost2*(2-score))/2;
}

/*
 * cost2 unless previous character was a space (' ' or SPACE_CHAR).
 * Note the & 255, necessary since chars might be signed and SPACE_CHAR
 * is in the high (negative) half, but c is an int in the range -1..255.
 */
HeapCost
FilterFollowsSpace(struct ParseNode *parent, char const *limit,
	struct Substitution const *subst)
{
	int c = OutputPrevChar(parent);
	(void)limit;
	return (c == ' ' || c == (SPACE_CHAR & 255)) ? subst->cost : subst->cost2;
}

/* cost2 unless previous character was duplicate of this one */
HeapCost
FilterAfterRepeat(struct ParseNode *parent, char const *limit,
	struct Substitution const *subst)
{
	int c = OutputPrevChar(parent);
	(void)limit;
	return (c == subst->output[0]) ? subst->cost : subst->cost2;
}

/* cost2 unless probably the closing quote in a char constant */
HeapCost
FilterCharConst(struct ParseNode *parent, char const *limit,
	struct Substitution const *subst)
{
	OutputHandle oh;
	int c;

	(void)limit;
	OutputInit(&oh, parent, NULL);
	c = OutputGetPrev(&oh);
	c = OutputGetPrev(&oh);
	if (c == '\\')
		c = OutputGetPrev(&oh);
	return (c == '\'') ? subst->cost : subst->cost2;
}

/*
 * If the identifier leading up to the current position contains
 * an underscore, then it's likely the current position is an underscore
 * as well; return cost.  If it does not, it's less likely; return cost2.
 */
HeapCost
FilterLikelyUnderscore(struct ParseNode *parent, char const *limit,
	struct Substitution const *subst)
{
	OutputHandle oh;
	int c;

	(void)limit;
	OutputInit(&oh, parent, NULL);
	for (;;) {
		c = OutputGetPrev(&oh);
		if (c == '_')
			return subst->cost;
		if (!isalnum(c))
			return subst->cost2;
	}
}

/* cost2 unless the following chars seem to be a checksum */
HeapCost
FilterChecksumFollows(struct ParseNode *parent, char const *limit,
	struct Substitution const *subst)
{
	int i, score = 0;
	char const *p = parent->input + subst->inlen;

	if (limit - p < PREFIX_LENGTH)
		return subst->cost2;
	if (!isspace((unsigned char)p[PREFIX_LENGTH-1]))
		return subst->cost2;
	for (i = 0; i < PREFIX_LENGTH-1; i++)
		score += (p[i] >= '0' && p[i] <= '9') + (p[i] >= 'a' && p[i] <= 'f');
	i = (score >= PREFIX_LENGTH-2 ? subst->cost : subst->cost2);
	/* Magic, since this function is perfect on binary files */
	if (i < COST_INFINITY && perpage.tabsize == 0)
		i = 0;
	return i;
}

/* Manage a *big* pool of ParseNodes */

struct MemPool nodePool;
struct ParseNode *nodeFreeList = 0;

/* Prepare for node allocations */
static void
NodePoolInit(void)
{
	memPoolInit(&nodePool);
	nodeFreeList = NULL;
}

/* Free all nodes in one swell foop */
static void
NodePoolCleanup(void)
{
	nodeFreeList = NULL;
	memPoolEmpty(&nodePool);
}

/* Allcoate a new (uninitialized) node */
static struct ParseNode *
NodeAlloc(void)
{
	struct ParseNode *node;

	node = nodeFreeList;
	if (node) {
		nodeFreeList = node->parent;
		return node;
	}
	return memPoolNew(&nodePool, ParseNode);
}

/* Free a node for reallocation */
static void
NodeFree(struct ParseNode *node)
{
	node->parent = nodeFreeList;
	nodeFreeList = node;
}

/*
 * Decrement a node's reference count, freeing it and
 * recursively decrementing its parent's if the count
 * goes to zero.
 */
static void
NodeRelease(struct ParseNode *node)
{
	struct ParseNode *parent;
	assert(node->refcnt);

	while (!--node->refcnt) {
		parent = node->parent;
		NodeFree(node);
		if (!parent)
			break;
		node = parent;
	}
}

/* Add nodes to the substitution tree */

/* Create a child of the given node, with the given properties. */
static ParseNode *
AddChild(ParseNode *parent, Substitution const *subst, HeapCost cost)
{
	ParseNode *child;

	if (cost == COST_INFINITY)
		return 0;

	cost += parent->cost;
	child = NodeAlloc();
	*child = *parent;
	/* Child is just like parent, except... */
	child->cost = cost;
	child->refcnt = 1;	/* The heap */
	child->input += subst->inlen;
	child->subst = subst;
	child->parent = parent;
	parent->refcnt++;
	return child;
}

/* Hash table of nasty lines, indexed by per-line CRC */
struct NastyLine {
	struct NastyLine *next;
	char const *line;
	CRC crc;
};

#define NASTY_HASH_SIZE 256
static struct NastyLine *nastyHash[NASTY_HASH_SIZE];	/* All zero */

struct MemPool nastyStrings, nastyStructs;
static CRCPoly const *nastyPoly = &crcCCITTPoly;
/*
 * Create a new NastyString entry if it doesn't already exist.
 * Note that this expects the string passed to end in a newline which
 * IS hashed but NOT stored
 */
static struct NastyLine *
AddNasty(char const *string)
{
	size_t len = strlen(string);	/* Including newline */
	CRC crc = CalculateCRC(nastyPoly, 0, (byte const *)string, len);
	struct NastyLine *nasty, **nastyp = nastyHash + (crc % NASTY_HASH_SIZE);
	char *line;

	/* Search for an existing copy */
	while ((nasty = *nastyp) != NULL) {
		if (nasty->crc == crc &&
			memcmp(nasty->line, string, len-1) == 0 &&
			nasty->line[len-1] == 0)
				return nasty;
		nastyp = &nasty->next;
	}
	/* Create a new structure */
	*nastyp = nasty = memPoolNew(&nastyStructs, struct NastyLine);
	nasty->next = NULL;
	nasty->line = line = memPoolAlloc(&nastyStrings, len, 1);
	nasty->crc = crc;
	memcpy(line, string, len-1);
	line[len-1] = 0;
	return nasty;
}

static void
RehashNasties(CRCPoly const *poly)
{
	struct NastyLine *cur, *head;
	CRC crc;
	int i;
	size_t len;

	/* Put everything into one list and clear the hash table */
	head = NULL;
	for (i = 0; i < (int)elemsof(nastyHash); i++) {
		while ((cur = nastyHash[i]) != NULL) {
			nastyHash[i] = cur->next;
			cur->next = head;
			head = cur;
		}
	}
	/* Recompute CRCs for the list and redistribute them among the buckets */
	while (head) {
		cur = head;
		head = head->next;
		len = strlen(cur->line);
		crc = CalculateCRC(poly, 0, (byte const *)cur->line, len);
		crc = AdvanceCRC(poly, crc, '\n');
		cur->crc = crc;
		cur->next = nastyHash[crc % NASTY_HASH_SIZE];
		nastyHash[crc % NASTY_HASH_SIZE] = cur;
	}
	nastyPoly = poly;
}

/* Read in the nastylines file */
static void
ReadNasties(FILE *f)
{
	char buf[128];

	while (fgets(buf, sizeof(buf)-1, f))
		AddNasty(buf);
}

/*
 * Convert an encoded string to binary.
 * No error checking is performed.
 */
static word32
GetWord32(EncodeFormat const *format, char const *buf, int len)
{
	word32 w = 0;

	while (len--)
		w = (w<<format->bitsPerDigit) + DecodeDigit(format, *buf++);
	return w;
}

/* Attempt nasty line substitutions */
static void
TryNasty(struct ParseNode *parent, Heap *heap, char const *limit)
{
	struct NastyLine const *nasty;
	struct Substitution const *subst;
	struct ParseNode *child;
	char const *end;
	EncodeFormat const *format = pnFormat(parent);
	OutputHandle oh;
	char buf[4];
	CRC check;
	int i;

	/* Make sure the lines are hashed properly */
	if (nastyPoly != format->lineCRC)
		RehashNasties(format->lineCRC);

	/* Get the line to be replaced */
	assert(parent->ps.pos == PREFIX_LENGTH);
	end = memchr(parent->input, '\n', limit - parent->input);
	if (!end)
		end = limit;
	/* Get the line's check value */
	OutputInit(&oh, parent, NULL);
	(void)OutputGetPrev(&oh);
	i = 4;
	while (--i)
		buf[i] = OutputGetPrev(&oh);
	check = GetWord32(format, buf, 4);
	/* Find the matches */
	nasty = nastyHash[check % NASTY_HASH_SIZE];
	for (; nasty; nasty = nasty->next) {
		if (nasty->crc == check) {
			subst = SubstNasty(parent->input, end-parent->input,
							   nasty->line, 0);
			if (subst) {
				child = AddChild(parent, subst, NASTY_COST);
				if (child) {
					child->ps.flags |= PS_FLAG_DYNAMIC;
					HeapInsert(heap, &child->cost);
				}
			}
		}
	}
}

/*
 * Form all of a ParseNode's children and add them to the heap.
 * Limit is the limit of allowable lookahead.
 */
static void
AddChildren(ParseNode *parent, Heap *heap, char const *limit)
{
	char c = parent->input[0];
	Substitution *subst = substitutions[0][c & 255];
	ParseNode *child;
	HeapCost cost;

/* If you want to make pure insertion substitutions, do that here */

	assert(parent->input < limit);	/* We always have at least one char */

	while (subst) {
		if (subst->inlen == 1 ||	/* Easy case */
			((size_t)(limit-parent->input) >= subst->inlen &&
			 memcmp(subst->input, parent->input, subst->inlen) == 0))
		{
			cost = subst->cost;
			if (subst->filter)
				cost = subst->filter(parent, limit, subst);
			child = AddChild(parent, subst, cost);
			if (child)
				HeapInsert(heap, &child->cost);
		}
		subst = subst->next;
	}

	/* Whole-line substitutions */
	if (parent->ps.pos == PREFIX_LENGTH)
		TryNasty(parent, heap, limit);
}


/* cost if this line has a dynamic substitution, otherwise cost2 */
HeapCost
FilterIsDynamic(struct ParseNode *parent, char const *limit,
	struct Substitution const *subst)
{
	(void)limit;
	return (parent->ps.flags & PS_FLAG_DYNAMIC) ? subst->cost : subst->cost2;
}

/* cost if the current page is binary mode, else cost2 */
HeapCost
FilterIsBinary(struct ParseNode *parent, char const *limit,
	struct Substitution const *subst)
{
	(void)parent; (void)limit;
	return perpage.tabsize ? subst->cost2 : subst->cost;
}

/* Debugging utility */
#define DEBUG 1	/* Set to 1 to print every line considered */

static size_t lastlen = 0;

static void
OverstrikeLine(char const *line, size_t len)
{
	static size_t lastlen = 0;
	int blanklen;

	if (!line) {
		if (lastlen)
			putchar('\n');
		lastlen = 0;
	} else if (len || lastlen) {
		if (len > 79)
			len = 79;
		blanklen = (lastlen > len) ? (int)lastlen - len : 0;
		printf("%.*s%*s\r", (int)len, line, blanklen, "");
		fflush(stdout);
		lastlen = len;
	}
}

/* Print everything, for debugging */
static void
PrintLine(char const *line, size_t len)
{
	if (line) {
		printf("%.*s\n", (int)len, line);
		lastlen = 0;
	}
}

static HeapCost ParseAdvanceString(Heap *heap, ParseNode *pn);

/*
 * Copy the parsechain from tail up to root, and hang it off of
 * newroot, adjusting the costs and parse state accordingly.  Returns
 * NULL if it is unable to (invalid parse, too expensive, etc.)
 * Note that as per the convention, ParseAdvanceString is *not* called
 * on the new tail node (but is called on all its parents).
 */
static ParseNode *
CopyParse(ParseNode const *tail, ParseNode const *root, ParseNode *newroot)
{
	ParseNode *newtail, *parent;

	if (tail == root)
		return newroot;
	parent = CopyParse(tail->parent, root, newroot);
	if (!parent)
		return NULL;
	newtail = AddChild(parent, tail->subst, ParseAdvanceString(NULL, parent));
	NodeRelease(parent);
	return newtail;
}

/*
 * Replace oldnode with a dynamic substitution for newchar, if possible,
 * and fill in the chain down to "tail" just like before, but with no branches.
 * Add the resultant ParseNode to the heap.
 */
static void
AddDynamic(Heap *heap, ParseNode const *oldnode, ParseNode const *tail,
	int newchar)
{
	Substitution const *subst = oldnode->subst;
	ParseNode *newnode;

	/* Only replace one-character substitutions */
	if (subst->outlen != 1)
		return;

	subst = SubstDynamic(oldnode->subst->input, SubstString(newchar), 0);
	newnode = AddChild(oldnode->parent, subst, -1); /* Try it immediately */
	if (newnode) {
		newnode->ps.flags |= PS_FLAG_DYNAMIC;
		newnode = CopyParse(tail, oldnode, newnode);
		if (newnode)
			HeapInsert(heap, &newnode->cost);
	}
}

/*
 * Do the same, at a given (1-based) position on the line.  Owing to
 * a minor glitch, we must never count the tail node, as this has not
 * been parsed yet, so its oldnode->ps.pos field is inaccurate.
 */
static void
AddDynamicAt(Heap *heap, int position, ParseNode const *tail, int newchar)
{
	ParseNode const *oldnode = tail;

	do {
		oldnode = oldnode->parent;
	} while (oldnode->ps.pos > position);

	if (oldnode->ps.pos == position)
		AddDynamic(heap, oldnode, tail, newchar);
}

/*
 * Given the computed and input check fields, correct the header field
 * that *ends* at the given pos.  This can be used for both the line and
 * page CRC errors by jyst changing the pos.  (It relies on the fact
 * that the page CRC fragment fits into the LineCRC type.)
 * It also relies on the fact that the CRC is at most 4 digits.
 */
static void
ErrorCorrectHeader(Heap *heap, ParseNode const *tail, int pos,
	EncodeFormat const *format, CRC crc, CRC check)
{
	CRC syndrome = crc ^ check;

	/* Find the position and the crc digit at that position */
	while (syndrome >= (CRC)format->radix) {
		if (syndrome & (CRC)(format->radix - 1))
			return;	/* uncorrectable */
		pos--;
		crc >>= format->bitsPerDigit;
		syndrome >>= format->bitsPerDigit;
	}
	/* Paste in the correct digit */
	AddDynamicAt(heap, pos, tail, EncodeDigit(format, crc & (format->radix-1)));
}

/*
 * This function walks back through the line, and if the line CRC could be
 * made correct by changing a character to another legal character,
 * the change is added (on probation) to the substitution table.
 */
static void
ErrorCorrect(Heap *heap, OutputHandle oh, EncodeFormat const *format,
	CRC syndrome)
{
	ParseNode const *tail = oh.node;
	int c;

	syndrome = ReverseCRC(format->lineCRC, syndrome, 0);
	while (oh.node->ps.pos > PREFIX_LENGTH) {
		c = OutputGetPrev(&oh);
		if (c == '\n' || c == -1) {	/* Can't happen */
			printf("Line ended at pos %d\n", oh.node->ps.pos);
			return;
		}
		syndrome = ReverseCRC(format->lineCRC, syndrome, 0);
		if (syndrome >= 0x100 || !substitutions[0][c^syndrome] ||
			oh.node->subst->outlen != 1)
				continue;
		AddDynamic(heap, oh.node, tail, c^syndrome);
	}
}

/*
 * Parsing operations.  This is a rather ugly and ad-hoc parser that
 * knows a lot about the fixed-field format produced by the munge
 * utility.  The main state variable is the position in
 * the line, which controls the expected header, the position of
 * tab stops, and the maximum permissible line length.
 */
#define OCCASIONALLY 100

/* Set up a ParseState to top-of-page */
static void
ParseStateInit(ParseState *ps)
{
	static struct ParseState const parseNull = { 0, 0, 0 };
	*ps = parseNull;
}

/*
 * Try to accept a newline, checking CRCs and even doing error-correction
 * as appropriate.
 */
static int
ParseNewline(Heap *heap, ParseNode *pn, char const *string)
{
	OutputHandle oh;
	int c;
	char debugbuf[PREFIX_LENGTH+LINE_LENGTH+10];
	char *header, *body, *end;
	int pos, width;
	CRC crc, check;
	ParseNode *temp;
	static int occasionally = OCCASIONALLY;
	EncodeFormat const *format = pnFormat(pn);
	EncodeFormat const *headerFormat = &hexFormat;

	/* Get the line into a buffer for analysis */
	OutputInit(&oh, pn, string);
	end = debugbuf + sizeof(debugbuf)-1;
	header = OutputGetUntil(oh, end, '\n');
	/* Strip leading and trailing whitespace */
	while (header < end && isspace((unsigned char)header[0]))
		header++;
	while (header < end && isspace((unsigned char)end[-1]))
		end--;
	*end++ = '\n';

	/* Start of checksummed area */
	body = header + PREFIX_LENGTH;
	/* Blank lines are missing the trainign space from the prefix */
	if (body >= end)
		body = end-1;

	crc = CalculateCRC(format->lineCRC, 0, body, end-body);
	check = GetWord32(format, header+2, 4);
	if (crc != check) {
		if (!--occasionally) {
			OverstrikeLine(header, end-header-1);
			occasionally = OCCASIONALLY;
		}
		/* Try ECC on the line */
		/* If we haven't already tried ECC on the line... */
		if (!(pn->ps.flags & PS_FLAG_DYNAMIC)) {
			ErrorCorrectHeader(heap, pn, PREFIX_LENGTH-1, format, crc, check);
			ErrorCorrect(heap, oh, format, crc ^ check);
		}
		return COST_INFINITY;
	}
	/* Good enough that we always print it */
	OverstrikeLine(header, end-header-1);

	/* Okay, now there are two cases - header line or running CRC */
	if (pn->ps.flags & PS_FLAG_INHEADER) {
		/* Do things for first header */
		if (!(pn->ps.flags & PS_FLAG_PASTHEADER)) {
			/* Check version number */
			width = EncodedLength(headerFormat, HDR_VERSION_BITS);
			c = (int)GetWord32(&hexFormat, body, width);
			if (c != 0) {
				fputs("Fatal: you need a newer version of repair"
				      " to process this file\n", stderr);
				exit(1);
			}
			/* Suck in page CRC, after version & flags */
			pos = width + EncodedLength(headerFormat, HDR_FLAG_BITS);
			width = EncodedLength(headerFormat, format->pageCRC->bits);
			perpage.page_check = GetWord32(&hexFormat, body+pos, width);
			/* Get tab size */
			pos += width;
			width = EncodedLength(headerFormat, HDR_TABWIDTH_BITS);
			perpage.tabsize = GetWord32(&hexFormat, body+pos, width);

			/* Once we have the header, don't reconsider */
			if (!(pn->ps.flags & PS_FLAG_PASTHEADER))
				while ((temp = (ParseNode *)HeapGetMin(heap)) != NULL)
					NodeRelease(temp);
			pn->ps.page_crc = 0;	/* Clear for top of page */
		}
	} else {
		/* Check the CRC-32 */
		crc = CalculateCRC(format->pageCRC, pn->ps.page_crc, body, end-body);
		pn->ps.page_crc = crc;
		crc = RunningCRCFromPageCRC(format, crc);
		check = GetWord32(format, header, 2);
		if (crc != check) {
			if (!(pn->ps.flags & PS_FLAG_DYNAMIC))
				ErrorCorrectHeader(heap, pn, 2, format, crc, check);
			return COST_INFINITY;
		}
	}

	/* Hey, it's correct! */
	PrintLine(header, end-header-1);

	/* Start next line */
	pn->ps.pos = 0;
	/* Clear most other flags, but we *have* got a header */
	c = pn->ps.flags & PS_FLAG_DYNAMIC;
	pn->ps.flags &= PS_FLAG_BINEND | PS_MASK_FORMAT;
	pn->ps.flags |= PS_FLAG_PASTHEADER;
	/*
	 * Give a bonus to the next line for having completed this one,
	 * less if it was dynamically fixed.
	 */
	return c ? COST_LINE : COST_LINE*2/3;
}

/*
 * Advance the parse state with pointed-to character.  Returns
 * COST_INFINITY if an impossible state is reached, otherwise returns a
 * cost value.  (Normally 0, this can be increased to penalize unlikely
 * output combinations to nudge the correction in a certain direction.)
 */
static HeapCost
ParseAdvance(Heap *heap, ParseNode *pn, char const *string)
{
	int i, retval = 0;
	char c = *string;
	EncodeFormat const *format = pnFormat(pn);

	/*
	 * Insist on spaces being correctly converted to SPACE_CHAR.
	 * There's a little irregularity just before EOL.
	 * Line contiunation and formfeed are also only legal at EOL.
	 */
	if (c == ' ') {
		if (pn->ps.flags & PS_FLAG_SPACE && !(pn->ps.flags & PS_FLAG_TAB))
			pn->ps.flags |= PS_FLAG_EOL;
		pn->ps.flags |= PS_FLAG_SPACE;
	} else if (pn->ps.flags & PS_FLAG_EOL) {
		if (c != '\n')
			return COST_INFINITY;
	} else if (c == SPACE_CHAR) {
		if (!(pn->ps.flags & PS_FLAG_SPACE))
			pn->ps.flags |= PS_FLAG_EOL;
	} else if (c == CONTIN_CHAR || c == FORMFEED_CHAR) {
			pn->ps.flags |= PS_FLAG_EOL;
	} else {
		pn->ps.flags &= ~PS_FLAG_SPACE;
	}

	switch (pn->ps.pos) {
		case 0:
			if (c == ' ' || c == '\n') {
				break;		/* Ignore ws and blank lines completely */
			} else if (c == '\f' || c == HDR_PREFIX_CHAR) {
				/* Start of a new page */
				pn->ps.flags |= PS_FLAG_INHEADER;	/* Expect header next */
				if (c == '\f')
					break;
				/* And fall through to increment pos */
			} else if (pn->ps.flags & PS_FLAG_INHEADER ||
					   pn->ps.flags & PS_FLAG_BINEND ||
					   !(pn->ps.flags & PS_FLAG_PASTHEADER) ||
					   DecodeDigit(format, c) < 0) {
				return COST_INFINITY;	/* Various illegal cases */
			}
			pn->ps.pos++;
			break;
		case 1:
			if ((pn->ps.flags & PS_FLAG_INHEADER)) {
				format = FindFormat(c);	/* Second char of header */
				if (!format)
					return COST_INFINITY;
				i = registerFormat(format);
				psSetFormat(&pn->ps, i);
				pn->ps.pos++;
				break;
			}
			if (DecodeDigit(format, c) < 0)
				return COST_INFINITY;	/* Illegal */
			pn->ps.pos++;
			break;
		case 2:
		case 3:
		case 4:
#if PREFIX_LENGTH != 7
#error fix this code
#endif
		case PREFIX_LENGTH-2:
			if (DecodeDigit(format, c) < 0)
				return COST_INFINITY;	/* Illegal */
			pn->ps.pos++;
			break;
		case PREFIX_LENGTH-1:
			if (c == ' ') {
				pn->ps.pos++;
				break;
			} else if (c != '\n') {
				return COST_INFINITY;
			}
			/* Blank lines may be missing this space char */
			/*FALLTHROUGH*/
		/* The normal line starts here, at position 7 */
		default:
			if (pn->ps.flags & PS_FLAG_INHEADER) {	/* Header line */
				/* Format is "--abcd 0123456789abcdef012 Page %u of %s" */
				int off = pn->ps.pos - (PREFIX_LENGTH+HDR_ENC_LENGTH);
				/* Offset relative to end of hex header */
				if (off < 0) {
					if (HexDigitValue(c & 255) < 0)
						return COST_INFINITY;
				} else if (off < 6) {
					if (c != " Page "[off])	/* Yes, this is legal C */
						return COST_INFINITY;
				} else if (off == 6) {
					if (c < '1' || c > '9')	/* First digit of page no. */
						return COST_INFINITY;
				} else {
					/* Re-base to end of scanned part of page number */
					off -= 7 + PageNumDigits(pn);
					if (off == 0) {
						if (c >= '0' && c <= '9' && PageNumDigits(pn) < 3)
							PageNumDigitsIncrement(pn);
						else if (c != ' ')
							return COST_INFINITY;
					} else if (off < 4) {
						if (c != " of "[off])
							return COST_INFINITY;
					} else if (off == 4) {
						if (!isgraph(c))
							return COST_INFINITY;
					} else if (c < ' ' || (c & 255) > '~') {
						if (c != '\n')
							return COST_INFINITY;
						return ParseNewline(heap, pn, string);
					}
				}
			} else if (!perpage.tabsize) {	/* Radix-64 line */
				/* Line is "RlNFVF9UQU==   \n" */
				if (isspace(c & 255)) {
					if (!(pn->ps.flags & PS_FLAG_BINWS)) {
						if ((pn->ps.pos - PREFIX_LENGTH) % 4 != 0)
							return COST_INFINITY;
						pn->ps.flags |= PS_FLAG_BINWS;
						if (pn->ps.pos - PREFIX_LENGTH < BYTES_PER_LINE*4/3)
							pn->ps.flags |= PS_FLAG_BINEND;
					}
					if (c == '\n')
						return ParseNewline(heap, pn, string);
				} else if (pn->ps.flags & PS_FLAG_BINWS) {
					return COST_INFINITY;
				} else if (c == RADIX64_END_CHAR) {
					if ((pn->ps.pos - PREFIX_LENGTH) % 4 < 2)
						return COST_INFINITY;
					pn->ps.flags |= PS_FLAG_BINEND;
				} else if (pn->ps.flags & PS_FLAG_BINEND) {
					return COST_INFINITY;
				} else if (Radix64DigitValue(c) < 0) {
					return COST_INFINITY;
				}
			} else {	/* Normal line */
				/* Make sure tab stops come out right */
				if (pn->ps.flags & PS_FLAG_TAB) {
					if (((pn->ps.pos - PREFIX_LENGTH) % perpage.tabsize) == 0)
						pn->ps.flags &= ~PS_FLAG_TAB;
					else if (c != TAB_PAD_CHAR && c != '\n') {
						return COST_INFINITY;	/* Illegal */
					}
				}
				/*
				 * Yes, this code has hard-coded ASCII assumptions
				 * It knows that the acceptable range of '\n', ' '..'~',
				 * TAB_CHAR, FORMFEED_CHAR is in that order.
				 * Signed char machines have it backwards, to be confusing.
				 */
				if ((c & 255) < ' ') {
					/* Newline! (Or something illegal) */
					if (c != '\n')
						return COST_INFINITY;
					return ParseNewline(heap, pn, string);
				}
				/* A normal character */
				if ((c & 255) > '~') {
					if (pn->ps.flags & PS_FLAG_INHEADER)
						return COST_INFINITY;	/* Illegal */
					if (c == TAB_CHAR)
						pn->ps.flags |= PS_FLAG_TAB;
					else if (c != FORMFEED_CHAR && c != SPACE_CHAR &&
												   c != CONTIN_CHAR)
						return COST_INFINITY;	/* Illegal */
				}
			}
			if (++pn->ps.pos > PREFIX_LENGTH + LINE_LENGTH)
				return COST_INFINITY;
			break;
	}
	return retval;
}

/*
 * Run the parser over the string in a ParseNode (using repeated calls
 * to ParseAdvance).  Return the penalty cost, or COST_INFINITY if
 * it's impossible
 */
static HeapCost
ParseAdvanceString(Heap *heap, ParseNode *pn)
{
	HeapCost cost, total = 0;
	char const *string = pn->subst->output;

	while (*string) {
		cost = ParseAdvance(heap, pn, string++);
		if (cost == COST_INFINITY)
			return cost;
		total += cost;
	}
	return total;
}

static unsigned int *globalStats = NULL;
static unsigned globalSize = 0;
static unsigned globalEdits = 0;

/*
 * This walks the list of substitutions, performing two tasks with
 * the statistics gathered.
 *
 * First, although not essential, it prints any interesting changes
 * (non-identity substitutions) made, and a count of the total number
 * of substitutions (including identity) as an approximate character count.
 *
 * Second, it does maintenance on dynamic (learned during program
 * execution) substitutions.  It discards any substitutions that end
 * up unused, and computes nice costs for the others, based on the
 * global (per-file) statistics.
 *
 * (This function is also called at the end to print the per-file stats,
 * which does redundant weight adjustment, but it's harmless.)
 */
static void
UseStats(unsigned int *stats, FILE *log)
{
	unsigned int i, j, n, changes = 0;
	unsigned long grand = 0;
	Substitution *s, **sp;

	if (!stats)
		return;

	/* Yes, this loop is permuted on purpose */
	for (j = 0; j < elemsof(*substitutions); j++) {
		for (i = 0; i < elemsof(substitutions); i++) {
			sp = &substitutions[i][j];
			while ((s = *sp) != 0) {
				grand += n = stats[s->index];
				/* Retain or purge dynamic substitutions, depending. */
				if (SubstIsDynamic(s)) {
					if (n) {
						SubstAdjust(s, n);
					} else if (!globalStats[s->index]) {
						/* Forget unused dynamic substitutions */
						*sp = s->next;
						if (SubstIsNasty(s))
							free((char *)s->input);	/* Dynamically allocated */
						SubstFree(s);
						continue;
					}
				}
				sp = &s->next;
				/*
				 * Print interesting substitutions.  Some boring substitutions,
				 * flagged with an index value of zero, are not printed.
				 */
				if (!s->index || !n)
					continue;
				changes += n;
				fprintf(log, "\t%2ux \"%.*s\"%*s-> \"%.*s\"%*s(cost ",
					   stats[s->index], (int)s->inlen, s->input,
					   s->inlen>3 ? 0 : 3-(int)s->inlen, "",
					   (int)s->outlen, s->output,
					   s->outlen>3 ? 0 : 3-(int)s->outlen, "");
				fprintf(log, s->cost == COST_INFINITY ? "-" : "%d", s->cost);
				if (s->filter)
					fprintf(log, s->cost2 == COST_INFINITY ? "/-" : "/%d",
					        s->cost2);
				fputs(SubstIsDynamic(s) ? ") ** LEARNED **\n" : ")\n", log);
			}
		}
	}
	fprintf(log, "\tTotal: %u changes (out of %lu)\n", changes, grand);
}

static void
DoStats(ParseNode const *node, unsigned int page, FILE *log)
{
	unsigned int *stats;
	unsigned int n;

	/* Enlarge global stats if needed */
	if (globalSize < substCount) {
		stats = realloc(globalStats, substCount * sizeof(*stats));
		if (!stats)  {
			fputs("Fatal error: out of memory for stats!\n", stderr);
			exit(1);
		}
		for (n = globalSize; n < substCount; n++)
			stats[n] = 0;
		globalStats = stats;
		globalSize = substCount;
	}

	/* Allocate per-page stats */
	stats = calloc(substCount, sizeof(*stats));
	if (!stats) {
		fputs("Fatal error: out of memory for stats!\n", stderr);
		exit(1);
	}
	/* Cheat and assume that calloc() initializes unsigned ints to zero */
	while (node) {
		stats[node->subst->index]++;
		node = node->parent;
	}

	/* Keep the global counts accurate */
	for (n = 0; n < substCount; n++)
		globalStats[n] += stats[n];

	fprintf(log, "Page %u substitutions:\n", page);
	UseStats(stats, log);

	free(stats);
}

/* Spit out a page of data (needs work).  Returns number of lines */
static unsigned
PrintPage(OutputHandle oh, FILE *out)
{
	char pagebuf[PAGE_BUFFER_SIZE];
	char *p1;	/* Beginning of current line */
	char *p2;	/* End of current line (WS stripped) */
	char *p3;	/* End of current line (newline) */
	char *p4;	/* End of all output */
	unsigned lines = 0;

	p4 = pagebuf + sizeof(pagebuf);
	p1 = OutputGetUntil(oh, p4, -1);

	/* Output the lines without leading & trailing whitespace */
	while (p1 < p4) {
		/* Identify the line */
		p3 = memchr(p1, '\n', p4-p1);
		if (!p3)
			p3 = p4;
		/* Delete leading whitespacee */
		while (isspace((unsigned char)*p1) && p1 < p3)
			p1++;
		/* Delete trailing whitepace */
		p2 = p3;
		while (isspace((unsigned char)p2[-1]) && p1 < p2)
			p2--;
		/* Spit out this line */
		fwrite(p1, 1, (size_t)(p2-p1), out);
		putc('\n', out);
		/* Advance p1 past the newline */
		p1 = p3 + 1;
		lines++;
	}
	return lines;
}

static volatile int interrupt = 0;
static void (* volatile oldhandler)(int) = SIG_DFL;

static void inthandler(int sig)
{
	if (++interrupt > 2)
		(void)signal(sig, oldhandler);
}

/*
 * Given a buffer, process a page from it and try to write a corrected page to
 * the out file.  Return the number of bytes accessed.  (0 if it was unable
 * to make any corrections.)
 */
static size_t
DoPage(char const *buf, size_t len, FILE *out, unsigned int page, FILE *log)
{
	ParseNode *node;
	Heap heap;
	HeapCost cost;
	OutputHandle oh;
	void (*sighandler)(int);

	HeapInit(&heap, 1000);
	NodePoolInit();
	PerPageInit(buf);

	/* Initialize signal handling */
	interrupt = 0;
	sighandler = signal(SIGINT, inthandler);
	if (sighandler != inthandler)
		oldhandler = sighandler;

	/* Make a root node */
	node = NodeAlloc();
	node->cost = 0;
	node->refcnt = 1;
	node->input = buf;
	node->subst = &substNull;
	ParseStateInit(&node->ps);
	node->parent = NULL;

	HeapInsert(&heap, &node->cost);

	/* The main loop: try to extend the current parse. */
	while ((node = (ParseNode *)HeapGetMin(&heap)) != NULL) {
		cost = ParseAdvanceString(&heap, node);
		if (cost != COST_INFINITY) {
			/* End of the file, or hit a second header line? */
			if (node->input == buf+len || InSecondHeader(&node->ps)) {
				/* Try to wrap up page, if page CRC works */
				if (node->ps.page_crc == perpage.page_check) {
					/* Success! */
					HeapDestroy(&heap);
					OutputInit(&oh, node, NULL);
					OverstrikeLine("", 0);

					if (InSecondHeader(&node->ps)) {
						/* Back up to last newline */
						OutputInit(&oh, node, NULL);
						while (OutputGetPrev(&oh) != '\n')
							;
						OutputUnget(&oh);
					}
					/* oh points to node that emitted last char on page */
					len = oh.node->input - buf; /* Chars eaten this page */
					perpage.lines = PrintPage(oh, out);
					DoStats(oh.node, page, log);
					NodePoolCleanup();
					return len;
				}
			} else {
				/* Keep working on the page */
				node->cost = cost += node->cost;
				if (node->input > perpage.maxpos) {
					perpage.maxpos = perpage.minpos = node->input;
					if (perpage.max_retries < perpage.retries)
						perpage.max_retries = perpage.retries;
					perpage.retries = 0;	/* Made progress */
				} else if (node->input < perpage.minpos) {
					perpage.minpos = node->input;	/* Furthest backtrack */
				}
				++perpage.retries;
				if (heap.numElems > MAX_HEAP || interrupt)
					HeapDestroy(&heap);
				else
					AddChildren(node, &heap, buf+len);
			}
		}
		NodeRelease(node);
	}
	/* Failed! */
	OverstrikeLine(NULL, 0);
	puts("Stopping for manual edit.");

	NodePoolCleanup();
	/* Get rid of the dynamic substitutions */
	DoStats(NULL, page, log);

	return 0;
}

/* The magic file-shuffling routine. */
static int
RepairFile(char const *name, char const *editor, char const *nastylines)
{
	char buf[PAGE_BUFFER_SIZE];
	char *filename;
	char const *p;
	size_t namelen;
	FILE *in = 0, *out = 0, *dump = 0, *log = 0;
	size_t inbytes;		/* Bytes in input buffer */
	size_t outbytes;	/* Bytes taken from input buffer */
	unsigned int pages = 0;	/* # of pages processed */
	unsigned int lines = 0;	/* # of lines processed (until trouble) */
	unsigned int minline, maxline;	/* Where is the error? */
	int giveup;			/* Have we had to abort corrections? */
	int err;			/* Copy of errno for returns */

	globalSize = 0;	/* Reset global stats */

	namelen = strlen(name);
	if (!(filename = malloc(namelen+10))) {
		p = "Unable to allocate memory\n";
		goto error;
	}

	memcpy(filename, name, namelen);
	strcpy(filename+namelen, ".log");
	puts(filename);
	if (!(log = fopen(filename, "at"))) {
		p = "Unable to open log file \"%s\"\n";
		goto error;
	}

	strcpy(filename+namelen, ".out");
	puts(filename);
	if (!(out = fopen(filename, "at"))) {
		p = "Unable to open output file \"%s\"\n";
		goto error;
	}

retry:
	/* Read in any new nasty lines */
	if (!(in = fopen(nastylines, "rt"))) {
		fprintf(stderr, "Unable to open nasty lines file \"%s\"\n", nastylines);
	} else {
		ReadNasties(in);
		fclose(in);
	}
	/* Try to open input file - .in or original */
	p = filename;
	strcpy(filename+namelen, ".in");
	if (!(in = fopen(filename, "rt"))) {
		if (!(in = fopen(name, "rt"))) {
			filename[namelen] = 0;
			p = "Unable to open input file \"%s\"\n";
			goto error;
		}
		p = name;
	}
	printf("Repairing from %s\n", p);
	strcpy(filename+namelen, ".dmp");
	if (!(dump = fopen(filename, "wt"))) {
		p = "Unable to open output file \"%s\"\n";
		goto error;
	}

	giveup = 0;
	inbytes = 0;	/* Bytes already at the front of the buffer */
	/* Append more data from the file */
	while ((inbytes += fread(buf+inbytes, 1, sizeof(buf)-inbytes, in)) != 0) {
		if (giveup) {
			/* Giving up mode - just copy through */
			outbytes = fwrite(buf, 1, inbytes, dump);
			if (!outbytes) {
				p = "Error writing dump file!\n";
				goto error;
			}
		} else {
			outbytes = DoPage(buf, inbytes, out, pages+1, log);
			NodePoolCleanup();
			if (outbytes) {
				pages++;
				lines += perpage.lines;
			} else {	/* Failed */
				/* Find range of backtracking for error location */
				minline = 1;
				for (p = buf;  p < perpage.minpos; p++)
					minline += (*p == '\n');
				for (maxline = minline; p < perpage.maxpos; p++)
					maxline += (*p == '\n');
				giveup = 1;
			}
		}
		/* Fewer bytes now in the buffer */
		inbytes -= outbytes;
		/* Move those bytes to the front again */
		memmove(buf, buf+outbytes, inbytes);
	}

	fclose(in);
	in = 0;
	fclose(dump);
	dump = 0;

	/* Okay, let's get tricky */
	memcpy(buf, name, namelen);
	strcpy(buf+namelen, ".dmp");
	strcpy(filename+namelen, ".in");

	/* teun: MS Visual C doesn't rename on top of existing file; remove it */
	if (remove(filename) != 0) {
		err = errno;
		fprintf(stderr, "Warning deleting %s\n", filename);
	}

	if (rename(buf, filename) != 0) {
		err = errno;
		fclose(out);
		fclose(log);
		/* teun: corrected buf, filename order. This cost me an hour */
		fprintf(stderr, "Error renaming %s -> %s\n", buf, filename);
		return err;
	}

	/* This code is spaghetti - is there a cleaner way? */
	if (giveup) {
		printf("Error in %s, lines %u-%u\n", filename, minline, maxline);
		fprintf(log, "Error in %s, lines %u-%u\n", filename, minline, maxline);
		if (interrupt > 1)
			goto manual;
		if (editor) {
			if (strcmp(editor, "-") == 0)
				goto manual;
			sprintf(buf, editor, maxline, filename);
		} else {
			p = getenv("VISUAL");
			if (!p)
				p = getenv("EDITOR");
			if (!p)
				goto manual;
			sprintf(buf, "%s +%u %s\n", p, maxline, filename);
		}
		printf("Executing %s\n", buf);
		globalEdits++;
		if (system(buf) == 0)
			goto retry;
		fputs("Edit failed - aborting\n", stderr);
manual:
		puts("Please fix the error by hand and re-run repair.");
	}

	fclose(out);
	free(filename);

	fprintf(log, "\n%u lines successfully processed.\n", lines);
	fprintf(log, "Overall substitutions (%u pages):\n", pages);
	UseStats(globalStats, log);
	printf("%u manual edits required\n", globalEdits);
	fclose(log);

	return 0;

error:
	err = errno;
	if (log) fclose(log);
	if (dump) fclose(dump);
	if (out) fclose(out);
	if (in) fclose(in);
	fprintf(stderr, p, filename);
	free(filename);
	return err;
}

/* Process the command line, calling RepairFile as needed. */
int
main(int argc, char *argv[])
{
	int		result = 0;
	int		i;
	char const *editor = NULL;
	char const *nastylines = "nastylines";

	InitUtil();
	SubstBuild();
	memPoolInit(&nastyStructs);
	memPoolInit(&nastyStrings);

	/* Process leading flags */
	for (i = 1; i < argc && argv[i][0] == '-'; i++) {
		if (argv[i][1] == '-' && argv[i][2] == 0) {
			i++;
			break;
		} else if (argv[i][1] == 'e') {
			editor = argv[i][2] ? argv[i]+2 : argv[++i];
		} else if (argv[i][1] == 'l') {
			nastylines = argv[i][2] ? argv[i]+2 : argv[++i];
		} else {
			editor = argv[i][2] ? argv[i]+2 : argv[++i];
			fprintf(stderr, "ERROR: Unrecognized option %s\n", argv[i]);
			return 1;
		}
	}

	/* Process files */
	for (; i < argc; i++) {
		result = RepairFile(argv[i], editor, nastylines);
		if (result != 0) {
			fprintf(stderr, "Fatal error: %s\n", strerror(result));
			return 1;
		}
	}

	return 0;
}

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
