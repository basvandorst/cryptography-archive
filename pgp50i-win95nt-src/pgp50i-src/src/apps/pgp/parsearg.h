/*
 * parsearg.h
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: parsearg.h,v 1.1.2.2 1997/06/07 09:49:07 mhw Exp $
 */

struct Flags {
	char const *outfile;
	char **recips;
	int numrecips;
	byte conventional;
	byte decrypt;
	byte doarmor;
	byte encrypt;
	byte filtermode;
	byte moreflag;
	byte sepsig;
	byte sign;
	byte wipe;
};

struct UIArg {
	struct PgpTtyUI	arg;
};

void mainParseEncryptArgs (struct PgpUICb *ui,
			   struct UIArg *ui_arg,
			   struct PgpEnv *env,
			   int *argcp,
			   char *argv[],
			   struct Flags *flags);

void mainParseSignArgs (struct PgpUICb *ui,
			struct UIArg *ui_arg,
			struct PgpEnv *env,
			int *argcp,
			char *argv[],
			struct Flags *flags);

void mainParseVerifyArgs (struct PgpUICb *ui,
			  struct UIArg *ui_arg,
			  struct PgpEnv *env,
			  int *argcp,
			  char *argv[],
			  struct Flags *flags);

void mainParse262Args (struct PgpUICb *ui,
		       struct UIArg *ui_arg,
		       struct PgpEnv *env,
		       int *argcp,
		       char *argv[],
		       struct Flags *flags);
