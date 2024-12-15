/*
 * If your compiler doesn't understand const (as does mips C version 1.32),
 * then you must
#define const
 * to get rid of it.
 */

const char	*const gmp_errstr[] = {
	/* 0 */		"No error",
	/* 1 */		"Out of memory",
	/* 2 */		"Internal error",
};
