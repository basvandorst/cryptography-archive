/*
 * pgpOpt.h -- A different sort of getopt()
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpOpt.h,v 1.2.2.1 1997/06/07 09:49:06 mhw Exp $
 *
 * Call pgpOptStart(opt, argc, argv) and then pgpOptNext(opt) until it returns
 * EOF.  pgpOptNext() returns either a flag character or 0 if there is
 * no flag.  opt->optarg is the string argument.  This is the argument
 * itself if pgpOptNext returns 0, or the following string ("foo" in "-xfoo"
 * or "-x foo") if pgpOptNext returns a character (e.g. 'x').
 *
 * If pgpOptNext returns a character, the caller must set optarg to 0
 * to indicate that the argument in opt->optarg has been claimed.
 * Otherwise, opt->optarg will be parsed by pgpOptNext the next time
 * it is called.  It will be parsed either as a flag (if the first
 * call got "-xfoo", the next call will return the flag 'f' with
 * optarg == "oo"), or as an argument (if the first call got "-x foo",
 * the next call will return 0 with optarg == "foo").
 *
 * A "boring" argument is one without a '-'.  For example "foo" is
 * boring, but "-foo" is not.
 *
 * State is a private variable with the following possible values:
 * -2 - All further arguments are "boring"; get a new one and return it.
 *      (This is entered if the option "--" is seen.)
 * -1 - Last argument was "boring"; get a new one and check for flags.
 *  0 - Last argument was a flag that ended a string (-o foo).  If opt->optarg
 *      is non-zero, examine "foo" as the next argument.  If it is zero,
 *      "foo" was claimed, so get a new one.
 *  1 - Last argument was a flag that did not end the string.  (-ofoo).
 *      If opt->optarg is non-zero, examine the tail "foo" for more flags.
 *      If it is zero, get a new argument.
 *
 * You may parse multi-letter options by examining opt->state, and if it
 * is greater than 0, opt->optarg follows with no intervening space.
 * (This lets you distinguish -abc [1] from -a bc [0].)
 *
 * You may then steal letters from the beginning of opt->optarg as needed.
 * Just perform "opt->optarg++" to steal them.  If you want to steal all
 * of opt->optarg, set opt->optarg = 0;
 */

#ifdef __cplusplus
extern "C" {
#endif

struct PgpOptContext {
	char *optarg;
	char **optargv;
	int optargc;
	int state;
};

void pgpOptStart (struct PgpOptContext *opt, int argc, char **argv);
int pgpOptNext (struct PgpOptContext *opt);

#ifdef __cplusplus
}
#endif
