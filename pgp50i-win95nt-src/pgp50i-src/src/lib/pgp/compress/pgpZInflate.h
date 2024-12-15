/*
* pgpZInflate.h - inflate a zip-compressed stream of bytes.
*
* This is a PRIVATE header file, for use only within the PGP Library.
* You should not be using these functions in an application.
*
* $Id: pgpZInflate.h,v 1.2 1996/12/19 20:26:34 mhw Exp $
*/

#include <stddef.h> /* For size_t */

#ifdef __cplusplus
extern "C" {
#endif

/* Private context structure */
struct InflateContext;

struct InflateContext *infAlloc(void);
void infFree(struct InflateContext *ctx);

/* Write bytes. Returns bytes written. error < 0 if trouble */
size_t infWrite(struct InflateContext *ctx, unsigned char const *buf,
 size_t len, int *error);
/* Get pointer & len of available output bytes */
unsigned char const *infGetBytes(struct InflateContext *ctx, unsigned *len);
/* Tell InflateContext that you've read "len" of available bytes */
void infSkipBytes(struct InflateContext *ctx, unsigned len);
/* Returns 0 for okay, or <0 for error */
int infEOF(struct InflateContext *ctx);

	#ifdef __cplusplus
	}
	#endif

	/* Error codes returned. */
	#define INFERR_NONE 0
	#define INFERR_NOMEM -1	/* Unable to allocate as needed */
	#define INFERR_BADINPUT -2	/* Corrupt input */
	#define INFERR_SHORT -3	/* Unexpected EOF */
	#define INFERR_LONG -4	/* Padding after expected EOF */
