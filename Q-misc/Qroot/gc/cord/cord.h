/* 
 * Copyright (c) 1993 by Xerox Corporation.  All rights reserved.
 *
 * THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 * OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 * Permission is hereby granted to copy this garbage collector for any purpose,
 * provided the above notices are retained on all copies.
 *
 * Author: Hans-J. Boehm (boehm@parc.xerox.com)
 */
 
/*
 * Cords are immutable character strings.  A number of operations
 * on long cords are much more efficient than their strings.h counterpart.
 * In particular, concatenation takes constant time independent of the length
 * of the arguments.  (Cords are represented as trees, with internal
 * nodes representing concatenation and leaves consisting of either C
 * strings or a functional description of the string.)
 *
 * The following are reasonable applications of cords.  They would perform
 * unacceptably if C strings were used:
 * - A compiler that produces assembly language output by repeatedly
 *   concatenating instructions onto a cord representing the output file.
 * - A text editor that converts the input file to a cord, and then
 *   performs editing operations by producing a new cord representing
 *   the file after echa character change (and keeping the old ones in an
 *   edit history)
 *
 * For optimal performance, cords should be built by
 * concatenating short sections.
 * This interface is designed for maximum compatibility with C strings.
 * ASCII NUL characters may be embedded in cords using CORD_from_fn.
 * This is handled correctly, but CORD_to_char_star will produce a string
 * with embedded NULs when given such a cord. 
 */
# ifndef CORD_H

# define CORD_H
# include <stddef.h>
# include <stdio.h>
/* Cords have type const char *.  This is cheating quite a bit, and not	*/
/* 100% portable.  But it means that nonempty character string		*/
/* constants may be used as cords directly, provided the string is	*/
/* never modified in place.  The empty cord is represented by, and	*/
/* can be written as, 0.						*/

typedef const char * CORD;

/* An empty cord is always represented as nil 	*/
# define CORD_EMPTY 0

/* Is a nonempty cord represented as a C string? */
#define IS_STRING(s) (*(s) != '\0')

/* Concatenate two cords.  If the arguments are C strings, they may 	*/
/* not be subsequently altered.						*/
CORD CORD_cat(CORD x, CORD y);

/* Concatenate a cord and a C string with known length.  Except for the	*/
/* empty string case, this is a special case of CORD_cat.  Since the	*/
/* length is known, it can be faster.					*/
CORD CORD_cat_char_star(CORD x, const char * y, size_t leny);

/* Compute the length of a cord */
size_t CORD_len(CORD x);

/* Cords may be represented by functions defining the ith character */
typedef char (* CORD_fn)(size_t i, void * client_data);

/* Turn a functional description into a cord. 	*/
CORD CORD_from_fn(CORD_fn fn, void * client_data, size_t len);

/* Return the substring (subcord really) of x with length at most n,	*/
/* starting at position i.  (The initial character has position 0.)	*/
CORD CORD_substr(CORD x, size_t i, size_t n);

/* Return the argument, but rebalanced to allow more efficient   	*/
/* character retrieval, substring operations, and comparisons.		*/
/* This is useful only for cords that were built using repeated 	*/
/* concatenation.  Guarantees log time access to the result, unless	*/
/* x was obtained through a large number of repeated substring ops	*/
/* or the embedded functional descriptions take longer to evaluate.	*/
/* May reallocate significant parts of the cord.  The argument is not	*/
/* modified; only the result is balanced.				*/
CORD CORD_balance(CORD x);

/* The following traverse a cord by applying a function to each 	*/
/* character.  This is occasionally appropriate, especially where	*/
/* speed is crucial.  But, since C doesn't have nested functions,	*/
/* clients of this sort of traversal are clumsy to write.  Consider	*/
/* the functions that operate on cord positions instead.		*/

/* Function to iteratively apply to individual characters in cord.	*/
typedef int (* CORD_iter_fn)(char c, void * client_data);

/* Function to apply to substrings of a cord.  Each substring is a 	*/
/* a C character string, not a general cord.				*/
typedef int (* CORD_batched_iter_fn)(const char * s, void * client_data);
# define CORD_NO_FN ((CORD_batched_iter_fn)0)

/* Apply f1 to each character in the cord, in ascending order,		*/
/* starting at position i. If						*/
/* f2 is not CORD_NO_FN, then multiple calls to f1 may be replaced by	*/
/* a single call to f2.  The parameter f2 is provided only to allow	*/
/* some optimization by the client.  This terminates when the right	*/
/* end of this string is reached, or when f1 or f2 return != 0.  In the	*/
/* latter case CORD_iter returns != 0.  Otherwise it returns 0.		*/
/* The specified value of i must be < CORD_len(x).			*/
int CORD_iter5(CORD x, size_t i, CORD_iter_fn f1,
	       CORD_batched_iter_fn f2, void * client_data);

/* A simpler version that starts at 0, and without f2:	*/
int CORD_iter(CORD x, CORD_iter_fn f1, void * client_data);
# define CORD_iter(x, f1, cd) CORD_iter5(x, 0, f1, CORD_NO_FN, cd)

/* Similar to CORD_iter5, but end-to-beginning.	No provisions for	*/
/* CORD_batched_iter_fn.						*/
int CORD_riter4(CORD x, size_t i, CORD_iter_fn f1, void * client_data);

/* A simpler version that starts at the end:	*/
int CORD_riter(CORD x, CORD_iter_fn f1, void * client_data);

/* Functions that operate on cord positions.  The easy way to traverse	*/
/* cords.  A cord position is logically a pair consisting of a cord	*/
/* and an index into that cord.  But it is much faster to retrieve a	*/
/* charcter based on a position than on an index.  Unfortunately,	*/
/* positions are big (order of a few 100 bytes), so allocate them with	*/
/* caution.								*/
/* Things in cord_position.h should be treated as opaque, except as	*/
/* described below.  Also note that					*/
/* CORD_pos_fetch, CORD_next and CORD_prev have both macro and function	*/
/* definitions.  The former may evaluate their argument more than once. */
# include "cord_position.h"

/*
	Visible definitions from above:
	
	typedef <OPAQUE but fairly big> CORD_pos[1];
	
	/* Extract the cord from a position:
	CORD CORD_pos_to_cord(CORD_pos p);
	
	/* Extract the current index from a position:
	size_t CORD_pos_to_index(CORD_pos p);
	
	/* Fetch the character located at the given position:
	char CORD_pos_fetch(register CORD_pos p);
	
	/* Initialize the position to refer to the give cord and index.
	/* Note that this is the most expensive function on positions:
	void CORD_set_pos(CORD_pos p, CORD x, size_t i);
	
	/* Advance the position to the next character.
	/* P must be initialized and valid.
	/* Invalidates p if past end:
	void CORD_next(CORD_pos p);
	
	/* Move the position to the preceding character.
	/* P must be initialized and valid.
	/* Invalidates p if past beginning:
	void CORD_next(CORD_pos p);
	
	/* Is the position valid, i.e. inside the cord?
	int CORD_pos_valid(CORD_pos p);
*/
# define CORD_FOR(pos, cord) \
    for (CORD_set_pos(pos, cord, 0); CORD_pos_valid(pos); CORD_next(pos))

			
/* An out of memory handler to call.  May be supplied by client.	*/
/* Must not return.							*/
extern void (* CORD_oom_fn)(void);

/* Dump the representation of x to stdout in an implementation defined	*/
/* manner.  Intended for debugging only.				*/
void CORD_dump(CORD x);

/* The following could easily be implemented by the client.  They are	*/
/* provided in cord_extras.c for convenience.				*/

/* Return the character in CORD_substr(x, i, 1)  	*/
char CORD_fetch(CORD x, size_t i);

/* Return < 0, 0, or > 0, depending on whether x < y, x = y, x > y	*/
int CORD_cmp(CORD x, CORD y);

/* Return a cord consisting of i ASCII NULs.  Dangerous in 		*/
/* conjunction with CORD_to_char_star.					*/
CORD CORD_nul(size_t i);

/* Turn a file into cord.  The file must be seekable.  Its contents	*/
/* must remain constant.  The file may be accessed as an immediate	*/
/* result of this call and/or as a result of subsequent accesses to 	*/
/* the cord.  Short files are likely to be immediately read, but	*/
/* long files are likely to be read on demand, possibly relying on 	*/
/* stdio for buffering.							*/
/* We must have exclusive access to the descriptor f, i.e. we may	*/
/* read it at any time, and expect the file pointer to be		*/
/* where we left it.  Normally this should be invoked as		*/
/* CORD_from_file(fopen(...))						*/
/* CORD_from_file arranges to close the file descriptor when it is no	*/
/* longer needed (e.g. when the result becomes inaccessible).		*/ 
CORD CORD_from_file(FILE * f);

/* Equivalent to the above, except that the entire file will be read	*/
/* and the file pointer will be closed immediately.			*/
CORD CORD_from_file_eager(FILE * f);

/* Equivalent to the above, except that the file will be read on demand.*/
CORD CORD_from_file_lazy(FILE * f);

/* Turn a cord into a C string.	The result shares no structure with	*/
/* x, and is thus modifiable.						*/
char * CORD_to_char_star(CORD x);

/* Write a cord to a file, starting at the current position.  No	*/
/* trailing NULs are newlines are added.				*/
/* Returns EOF if a write error occurs, 1 otherwise.			*/
int CORD_put(CORD x, FILE * f);

/* "Not found" result for the following two functions.			*/
# define CORD_NOT_FOUND ((size_t)(-1))

/* A vague analog of strchr.  Returns the position (an integer, not	*/
/* a pointer) of the first occurrence of (char) c inside x at position 	*/
/* i or later. The value i must be < CORD_len(x).			*/
size_t CORD_chr(CORD x, size_t i, int c);

/* A vague analog of strrchr.  Returns index of the last occurrence	*/
/* of (char) c inside x at position i or earlier. The value i		*/
/* must be < CORD_len(x).						*/
size_t CORD_rchr(CORD x, size_t i, int c);
# endif /* CORD_H */
