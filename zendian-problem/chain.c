/*
	chain -- chain together token sequences

	PUBLIC-DOMAIN implementation for any hosted C environment

	created:	01-Oct-1991	Gwyn@BRL.MIL
	last edit:	92/02/10	Gwyn@BRL.MIL
	SCCS ID:	@(#)chain.c	1.1

	Usage:	$ chain [ -d delims ] [ -w wildcard ] < input_file > output_file

	"input_file" contains one sequence per line, with tokens separated by
	one or more of a specified set of delimiters (default: space and tab);
	a reserved token (default: "", which amounts to no wild card) can be
	used as a "wild card" token.  As a special case, specifying an empty
	string for the delimiter argument causes each character to be treated as
	a separate token.

	"output_file" will contain maximal "chained" token sequences, one per
	line, delimited by the first specified delimiter (default: space).
	For any chain that forms a loop, its output sequence will begin at an
	arbitrary token in the loop, and the last token will be followed by a
	delimiter.

	If these input and output formats aren't quite what you would like, you
	can easily use "sed" and/or other standard utilities to convert them.

	Examples:	$ chain -w '?' <<!
			QUHW	MUCO	?	JUHQ	SAFA	LUBM
			TUSK	RIMD	?	MUCO	?	JUHQ
			!
			TUSK RIMD QUHW MUCO ? JUHQ SAFA LUBM

			$ chain -d '-' <<!
			b-c-d
			e-f-g
			a-b
			d-a
			!
			a-b-c-d-
			e-f-g

			$ chain -d '' <<!
			FGJ
			XYZ
			RTI
			PQS
			KEB
			ICH
			ZAR
			MNP
			VWX
			HOK
			SUV
			JLM
			BDF
			!
			ARTICHOKEBDFGJLMNPQSUVWXYZ

	See also:	tsort(1).
*/

#include	<stdio.h>
#include	<string.h>

#if __STDC__
#include	<stdlib.h>

typedef void		*pointer;	/* generic pointer */
#else
typedef char		*pointer;	/* generic pointer */
typedef unsigned	size_t;		/* type of result of "sizeof" */

extern void	exit();
extern pointer	malloc();

#define	EXIT_FAILURE	(-1)		/* (configure as required) */
/* #define	strrchr	rindex		/* (configure as required) */
#endif

#define	DEF_DELIMS	" \t"		/* default input token delimiters */
#define	DEF_WILDCARD	""		/* default wild-card token (none) */

static char		*delims = DEF_DELIMS;	/* input token delimiters */
static char		*wildcard = DEF_WILDCARD;	/* wild-card token */
static char		*progname;	/* program name, for diagnostics */
static unsigned long	linenum = 0L;	/* input line number */

/*
	Error handling:

	Separate functions are used to avoid having variable argument lists.
*/

static void
Fatal( mess )
	char	*mess;
	{
	(void)fprintf( stderr, "%s:", progname );

	if ( linenum != 0L )
		(void)fprintf( stderr, " line %lu:", linenum );

	(void)fprintf( stderr, " %s\n", mess );
	exit( EXIT_FAILURE );
	}

static void
Clash( s1, s2 )
	char	*s1, *s2;
	{
	(void)fprintf( stderr, "%s: line %lu: \"%s\" and \"%s\" conflict\n",
		       progname, linenum, s1, s2
		     );
	exit( EXIT_FAILURE );
	}

/*
	Memory allocation:

	Checks for successful memory allocation so its callers don't have to.
*/

static pointer
Alloc( nbytes )				/* checking interface to malloc() */
	size_t			nbytes;	/* desired allocation size, in bytes */
	{
	register pointer	ptr = malloc( nbytes );

	if ( ptr == NULL )
		Fatal( "out of memory" );

	return ptr;
	}

/*
	Token list node management:

	Each non-wild-card token name has a unique node in the collection of
	so-far maximal sequence lists.  Each wild-card token is assigned its own
	unique node.  The nodes in each maximal sequence are linked together via
	their "pred" and "succ" members, with null pointers marking the ends of
	every noncyclic sequence.

	In order to quickly locate an input token name among the existing nodes,
	non-wild-card nodes are also organized as a binary search tree via their
	"lt" and "gt" members.
*/

typedef struct node
	{
	char		*name;		/* -> token name (NULL for wild card) */
	struct node	*pred;		/* -> predecessor in sequence or NULL */
	struct node	*succ;		/* -> successor in sequence or NULL */
	struct node	*lt;		/* -> "less-than" sorted subtree */
	struct node	*gt;		/* -> "greater-than" sorted subtree */
	}	node;			/* node in token-sequence lists */

static node	*rootp = NULL;		/* -> root of token-name search tree */
static node	*freelist = NULL;	/* -> chain of freed nodes */

#define	BATCH	64			/* number of nodes allocated at once */

static node *
NodeAlloc()				/* efficiently allocate a node */
	{
	static int	nleft = 0;	/* number of nodes remaining in batch */
	static node	*batchp;	/* -> next node in allocated batch */
	register node	*np;		/* -> allocated node */

	if ( (np = freelist) != NULL )
		freelist = np->succ;	/* removing old head of free list */
	else	{
		if ( --nleft < 0 )
			{		/* allocate a fresh batch of nodes */
			batchp = (node *)Alloc( BATCH * sizeof(node) );
			nleft = BATCH - 1;
			}

		np = batchp++;		/* take node from current batch */
		}

	return np;
	}

static void
NodeFree( np )				/* deallocate a surplus node */
	node	*np;
	{
	/* Note that np->name is always NULL when NodeFree() is invoked. */

	np->succ = freelist;		/* avoid free()/malloc(), for speed */
	freelist = np;
	}

static node *
NodeP( name )				/* locate a token in the search tree */
	register char	*name;		/* -> input token name */
	{
	register node	**p;		/* -> root of search subtree */
	register int	k;		/* result of strcmp() */

	/* Look for token name in existing search tree. */

	for ( p = &rootp; *p != NULL; )
		if ( (k = strcmp( name, (*p)->name )) < 0 )
			p = &(*p)->lt;
		else if ( k == 0 )
			return *p;	/* -> existing node */
		else
			p = &(*p)->gt;

	/* Not found; enter a new node into the search tree. */

	*p = NodeAlloc();
	(*p)->name = strcpy( (char *)Alloc( strlen( name ) + sizeof(char) ),
			     name
			   );		/* much like strdup(name) */
	(*p)->lt = (*p)->gt = (*p)->pred = (*p)->succ = NULL;	/* isolated */
	return *p;
	}

/*
	Token sequence input, parsing, and merging:

	GetInput() reads a whole line into a buffer, then parses it into
	tokens, each of which (except for wild cards) is looked up in the set of
	sequence lists.  Each input token is evaluated in a context where all
	preceding tokens in the input sequence have already been properly merged
	into the set of sequence lists; thus, sequence merging consists simply
	of checking each current input token's list against the previous token's
	list, reporting conflicts, taking actions to merge separate lists, and
	replacing wild cards as their names become known.
*/

#define	MAX_STRLEN	511		/* maximum input line length */

static char	line[MAX_STRLEN + 1];	/* input line buffer */

static void
GetInput()				/* obtain next input sequence */
	{
	static char	buf[2] = { 0, '\0' };	/* name buffer when no delims */
	register node	*np;		/* -> current token's node */
	register node	*pp;		/* -> previous token's node */
	register char	*lp;		/* -> start of scan for a token */
	register char	*name = buf;	/* -> NUL-terminated token in buffer */

	while ( fgets( line, MAX_STRLEN + 1, stdin ) != NULL )
		{			/* read a line of text into buffer */
		++linenum;

		if ( (lp = strchr( line, '\n' )) == NULL )
			Fatal( "input line too long" );

		*lp = '\0';		/* remove new-line */

		/* Tokenize buffer, merging each token into existing chains. */

		for ( pp = NULL, lp = line;
		      delims[0] != '\0' && (name = strtok( lp, delims )) != NULL
		   || delims[0] == '\0' && (buf[0] = *lp) != '\0';
		      pp = np, lp = delims[0] != '\0' ? NULL : lp + 1
		    )
			if ( strcmp( name, wildcard ) == 0 )
				{
				if ( pp == NULL || (np = pp->succ) == NULL )
					{	/* new wild-card node */
					np = NodeAlloc();
					np->name = NULL;
					np->succ = NULL;

					if ( (np->pred = pp) != NULL )
						pp->succ = np;
					}
				/* else current node is previous's successor */
				}
			else	{
				register node	*a, *b;	/* -> along chains */
				register node	*ap, *bp;	/* -> pred */
				register node	*as, *bs;	/* -> succ */
				node		*ax, *bx;	/* start a, b */

				np = NodeP( name );	/* locate named node */

				if ( pp == NULL )
					continue;	/* no merging yet */

				/* The current node's chain is merged with the
				   previous node's chain as follows:  Separate
				   link pointers are maintained for the two
				   chains.  Start at corresponding nodes and
				   first move backward, reconciling each node,
				   then move forward, doing the same.  Cycles
				   do not require special handling, because
				   eventually both chain nodes will be the same
				   named node, which terminates the loop. */

				if ( (ax = pp->succ) != NULL )
					bx = np;
				else if ( (bx = np->pred) != NULL )
					ax = pp;
				else	{	/* join chains at their ends */
					pp->succ = np;
					np->pred = pp;
					continue;	/* no overlap */
					}

				/* Reconcile "upstream" portions of chains. */

				for ( a = ax, b = bx;
				      a != NULL && b != NULL && a != b;
				      a = ap, b = bp
				    )	{
					/* Remember where the neighbors are. */

					if ( (ap = a->pred) == NULL )
						a->pred = ap = b->pred;

					if ( (as = a->succ) == NULL )
						a->succ = as = b->succ;

					if ( (bp = b->pred) == NULL )
						b->pred = bp = ap;

					if ( (bs = b->succ) == NULL )
						b->succ = bs = as;

					/* Replace wild card, giving precedence
					   to the previously-merged chain. */

					if ( b->name == NULL )
						{
						if ( bs != NULL )
							bs->pred = a;

						if ( bp != NULL )
							bp->succ = a;

						if ( ax == b )
							ax = a;

						if ( bx == b )
							bx = a;

						NodeFree( b );
						}
					else if ( a->name == NULL )
						{
						if ( as != NULL )
							as->pred = b;

						if ( ap != NULL )
							ap->succ = b;

						if ( ax == a )
							ax = b;

						if ( bx == a )
							bx = b;

						NodeFree( a );
						}
					else	/* diff nodes => diff names */
						Clash( a->name, b->name );
					}

				/* Reconcile "downstream" portions of chains. */

				for ( a = ax, b = bx;
				      a != NULL && b != NULL && a != b;
				      a = as, b = bs
				    )	{
					/* Remember where the neighbors are. */

					if ( (ap = a->pred) == NULL )
						a->pred = ap = b->pred;

					if ( (as = a->succ) == NULL )
						a->succ = as = b->succ;

					if ( (bp = b->pred) == NULL )
						b->pred = bp = ap;

					if ( (bs = b->succ) == NULL )
						b->succ = bs = as;

					/* Replace wild card, giving precedence
					   to the previously-merged chain. */

					if ( b->name == NULL )
						{
						if ( bp != NULL )
							bp->succ = a;

						if ( bs != NULL )
							bs->pred = a;

						NodeFree( b );
						}
					else if ( a->name == NULL )
						{
						if ( ap != NULL )
							ap->succ = b;

						if ( as != NULL )
							as->pred = b;

						NodeFree( a );
						}
					else	/* diff nodes => diff names */
						Clash( a->name, b->name );
					}
				}
		}

	if ( ferror( stdin ) )
		Fatal( "input error" );

	/* else EOF; return */
	}

/*
	Final (chained) sequence output:

	Traverses the (non-wild-card) token tree, tracking down the start of the
	sequence when an unprinted token is found, then printing that whole
	sequence.  After each token is output, its name is set to NULL (same as
	for wild card) to prevent its triggering further printing.  (Note that
	each non-wild-card token occurs exactly once in the output.)

	The sequence containing the lexicographically first non-wild-card token
	will be printed first, then the sequence containing the lowest remaining
	unprinted token, and so on.  I'm not sure what real use can be made of
	this particular property, but there must be some application.

	This version uses recursion, which in case of nearly sorted input could
	nest deeply; if your system has a small run-time stack, you may need to
	change this.  For "random" tokens, this should seldom pose a problem.
*/

static void
DumpNode( p )
	register node	*p;		/* -> some token in a sequence */
	{
	register node	*p0;		/* -> first node in the sequence */

	/* Find beginning of chain containing the specified token. */

	for ( p0 = p; p0->pred != NULL && (p0 = p0->pred) != p; )
		;			/* watch out for closed cycle */

	/* Print each token in the chain, with delimiters as required. */

	p = p0;
	do	{
		if ( p->name == NULL )
			p->name = wildcard;	/* temporarily */

		if ( fputs( p->name, stdout ) == EOF )
			Fatal( "output error" );

		p->name = NULL;		/* ensure printing just once */
		p = p->succ;

		if ( p != NULL
		  && delims[0] != '\0' && putchar( delims[0] ) == EOF
		   )
			Fatal( "output error" );
		}
	while ( p != NULL && p != p0 );

	if ( putchar( '\n' ) == EOF )
		Fatal( "output error" );
	}

static void
DumpTree( np )
	register node	*np;
	{
	if ( np->lt != NULL )
		DumpTree( np->lt );

	if ( np->name != NULL )
		DumpNode( np );		/* "inorder" traversal */

	if ( np->gt != NULL )
		DumpTree( np->gt );
	}

static void
Dump()
	{
	if ( rootp != NULL )
		DumpTree( rootp );
	}

/*
	Option argument processing:

	For ease of porting, getopt() is not used.

	This version supports non-option arguments, for future expansion.
*/

static void
OptArgs( pargc, pargv )
	register int	*pargc;		/* -> argc */
	register char	***pargv;	/* -> argv */
	{
	/* Save "simple part" of argv[0] for use in diagnostic printing. */

	if ( (progname = strrchr( **pargv, '/' )) != NULL )
		++progname;
	else
		progname = **pargv;

	/* Process all options. */

	while ( --*pargc > 0 && **++*pargv == '-' )
		switch ( (**pargv)[1] )
			{
		case 'd':
			if ( --*pargc == 0 )
				Fatal( "missing delimiter argument" );

			delims = *++*pargv;	/* empty string okay */
			break;

		case 'w':
			if ( --*pargc == 0 )
				Fatal( "missing wild-card argument" );

			wildcard = *++*pargv;	/* empty string okay */
			break;

		case '-':
			return;		/* required by command interface std. */

		default:
			Fatal( "unsupported option" );
			}
	}

/*
	Control loop:

	All the hard work is done by subroutines.
*/

int
main( argc, argv )
	int	argc;
	char	**argv;
	{
	OptArgs( &argc, &argv );	/* handle option arguments */

	if ( argc > 0 )
		Fatal( "non-option argument not permitted" );

	GetInput();			/* merge input sequences into chains */
	Dump();				/* print the final chains */
	return 0;
	}
