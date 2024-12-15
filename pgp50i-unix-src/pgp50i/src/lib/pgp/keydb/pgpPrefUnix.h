/*
 * pgpPrefUnix.h -- Companion to pgpPrefUnix.c; most functions are not
 * exported, though.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Added 970514 by Brett A. Thomas (quark@baz.com, bat@pgp.com)
 *
 * $Id: pgpPrefUnix.h,v 1.1.2.2.2.1 1997/07/19 00:37:48 quark Exp $
 *
 */

/*Checks for the existance of the directory *fn. If it exists, nothing is
 *done. If it exists and is a file, the user is given an appropriate message.
 *If it doesn't exist, we tell the user we're creating it, create it, and
 *let the user know what's happening. This is ifdeffed Unix in an attempt
 *not to disrupt other platforms; it should work with little effort on DOS,
 *and should be useless for Windows and Mac apps.
 */

void CheckAndCreatePGPDir(const char *fn);

PGPError pgpSetDefaultPrefsInternal50Tmp(PgpEnv *env);

