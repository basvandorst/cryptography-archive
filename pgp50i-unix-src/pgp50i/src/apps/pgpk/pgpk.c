/*
 * pgpk.c -- PGP keyring manipulation program
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpk.c,v 1.13.2.12.2.19 1997/08/27 03:57:02 quark Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>		/* for sbrk() */
#endif
#ifdef UNIX
#include <sys/stat.h>		/* for umask() */
#endif

#include "pgpDebug.h"
#include "pgpEnv.h"
#include "pgpMem.h"
#include "pgpKeyDB.h"
#include "pgpOutput.h"
#include "pgpkUI.h"
#include "pgpUI.h"
#include "pgpInitApp.h"
#include "pgpExit.h"
#include "pgpUI.h"
#include "pgpOpt.h"
#include "pgpUserIO.h"
#include "pgpRndPool.h"
#include "pgpConf.h"

#include "pass.h"
#include "pgpkKeyGen.h"
#include "keyserver.h"
#include "url.h"

#include "pgpLicense.h"


#define KDBTYPE_KEY          1
#define KDBTYPE_USERID       2
#define KDBTYPE_CERT         3

#define MAXARGS 2

struct Flags {
	char args[MAXARGS];
	int argc;
	char const *outfile;
	char const *ringfile;
	char const *signName;
	PGPKey *signKey;
	char opt;
	byte doarmor;
    char quit;
};

PGPKeySet  *defaultset = NULL;    /* All keys in default keyrings */
Boolean		enableUI = TRUE;

#define PASSLEN 256

static Boolean GetFileFromServer(char *file)
{
	extern PgpEnv *pgpEnv;
	Boolean GetFile;

	GetFile = pgpenvGetInt(pgpEnv, 
						   PGPENV_AUTOSERVERFETCH, NULL, NULL);

	if(GetFile) {
		if(*file == '.' && *(file + 1) == '/') {
			GetFile = FALSE;
		}
		else {
			if(*file == '/') {
				GetFile = FALSE;
			}
		}
	}

	return(GetFile);
}


static void
setOptArgs (char const *args, struct PgpOptContext *opt, struct Flags *flags)
{
	while (*opt->optarg && strchr (args, *opt->optarg))
		flags->args[flags->argc++] = *(opt->optarg++);
}

static void
setOpt (int opt, struct PgpOptContext *optp, struct Flags *flags)
{
	if (!flags->opt) {
		flags->opt = opt;

		if (optp->state == 1 && optp->optarg)
			switch (opt) {
			case 'k':
				setOptArgs ("s", optp, flags);
				break;
			case 'l':
				setOptArgs ("l", optp, flags);
				break;
			case 'r':
				setOptArgs ("su", optp, flags);
				break;
			case 'x':
				setOptArgs ("a", optp, flags);
				break;
			}

		return;
	}
	ErrorOutput(TRUE, LEVEL_CRITICAL, "ARGS_INCOMPATABLE", flags->opt, opt);
}

/*
 * A key printer for selectOneDBObj (above),
 * which prints keys for a menu.
 */
static void
keyPrint(PGPKeyIter *iter, PgpOutputType OutputType)
{
	SpecifiedOutputString(FALSE, OutputType, 0, "\n");
	kdbTtyShowKey(OutputType, iter, defaultset, 0);
}

/*
 * A name printer for selectOneDBObj (above),
 * which prints keys for a menu.
 */
static void
namePrint(PGPKeyIter *iter,	PgpOutputType OutputType)
{
	char namestring[256];
	size_t len = 256;

	SpecifiedOutputString(FALSE, OutputType, 0, "\n");
	pgpGetUserIDString (pgpKeyIterUserID (iter), kPGPUserIDPropName,
						namestring, &len);
	kdbTtyPutString (namestring, len, (unsigned) len, TRUE, OutputType, 0, 0);
	SpecifiedOutputString(FALSE, OutputType, 0, "\n");
	kdbTtyShowSigs (OutputType, iter, defaultset, 2);
}

/*
 * A sig printer for selectOneDBObj (above),
 * which prints keys for a menu.
 */
static void
sigPrint(PGPKeyIter *iter, PgpOutputType OutputType)
{
	pgpAssert(defaultset);
	SpecifiedOutputString(FALSE, OutputType, 0, "\n");
	kdbTtyShowSig(OutputType, iter, defaultset, 1);
}


/*  Print the deepest current object */

static void
objPrint(PGPKeyIter *iter, PgpOutputType OutputType)
{
    PGPKeyIter  *printiter = pgpCopyKeyIter (iter);

	if (pgpKeyIterCert (printiter))
		sigPrint (printiter, OutputType);
	else if (pgpKeyIterUserID (printiter))
		namePrint (printiter, OutputType);
	else
		keyPrint (printiter, OutputType);
	pgpFreeKeyIter (printiter);
}

/*
 * Searches <fullset> for keys matching the substrings on the
 * command line.
 * If <argc>==0 and <defAll>, uses the entire <fullset>.
 * In either case, the *subset returned is a valid, newly created,
 * keyset.
 *
 * Returns 1 if any keys were selected, else 0.
 */
static int
selectKeyArgs(int argc, char *argv[], PGPKeySet *fullset,
			  PGPKeySet **subset, int defAll, Boolean KeySetReadOnly)
{
	PGPError AddKeysResult = PGPERR_OK;
	int AnyKeys = 0;

	if (argc <= 0 && defAll) {
		*subset = pgpCopyKeySet (fullset);
	} else {
		if(!KeySetReadOnly) {
			if(argc == 1) {
				*subset = pgpFilterKeySetAuto(fullset, argv[0]);
			}
			else {
				ErrorOutput(TRUE, LEVEL_CRITICAL, "ONLY_ONE_USER_ALLOWED");
				exitUsage (PGPEXIT_ARGS);
			}
		}
		else {
			int i = 0;
			PGPKeySet *SingleMatchSet;
			/*Assign subset to a new keyset.  We can't just use the result of
			 *pgpFilterKeySetAuto(), because it returns a read-only keyset
			 *(not like that's documented, or anything)
			 */
			*subset = pgpNewKeySet();

			/*Loop through the command line arguments, adding each one to the
			 *resulting keyset.
			 */
			for(i = 0;
				i < argc && *subset && AddKeysResult == PGPERR_OK;
				++i) {
				if((SingleMatchSet = pgpFilterKeySetAuto (fullset, argv[i]))) {
					AddKeysResult = pgpAddKeys(*subset, SingleMatchSet);
					pgpFreeKeySet(SingleMatchSet);
				}
			}
		}
	}

	if((AddKeysResult == PGPERR_OK) &&
	   *subset &&
	   (pgpCountKeys(*subset) > 0)) {
		AnyKeys = 1;
	}
	return (AnyKeys);
}

/*
 * Select one object of type <selecttype> and return an iterator pointing
 * to it.  The iterator passed is replaced by a new one.
 *
 * <targettype> defines the requirements for the returned object. For example,
 * if <selecttype> == key and <targettype> == sig, the selected key must
 * have a signed userid attached to it.
 */
static int
selectOneDBObj(PGPKeyIter **iter,
			   int selecttype,
			   int targettype,
			   char *header)
{
	PGPKeyIter		*selectiter = *iter, *targetiter = NULL;
	PGPKeyIter		*iterList[50];
	PGPKey			*key = NULL;
	PGPUserID		*userid = NULL;
	PGPCert			*cert = NULL;
	int             maxObjs = sizeof(iterList) / sizeof(iterList[0]);
	int             numObjs = 0;
	int             notFirst = 0;
	int             result = 0;
	long            choice;
	char            *endNum, buf[32];
	int             i;
	extern PgpEnv *pgpEnv;
	
	do {
		result = 0;
		
		switch (selecttype) {
			case KDBTYPE_KEY:
				key = pgpKeyIterNext (selectiter);
				if(key) {
					/*  Do we need a name or a sig on the key? */
					if (targettype == KDBTYPE_USERID ||
						targettype == KDBTYPE_CERT) {
						targetiter = pgpCopyKeyIter (selectiter);
						do {
							if((userid = pgpKeyIterNextUserID (targetiter)) &&
							   ((targettype == KDBTYPE_USERID) ||
								((targettype == KDBTYPE_CERT) &&
								(cert = pgpKeyIterNextUIDCert(targetiter)))))
								result = 1;
						}while(userid && !result);
						
						pgpFreeKeyIter (targetiter);
					}
					else
						result = 1;
				}
				break;

			case KDBTYPE_USERID:
				if((userid = pgpKeyIterNextUserID (selectiter))) {
					if (targettype == KDBTYPE_CERT) {
						targetiter = pgpCopyKeyIter (selectiter);
						cert = pgpKeyIterNextUIDCert (targetiter);
						if (cert)
							result = 1;
						pgpFreeKeyIter (targetiter);
					}
					else
						result = 1;
				}
				break;

			case KDBTYPE_CERT:
				cert = pgpKeyIterNextUIDCert (selectiter);
				if (cert)
					result = 1;
				break;
		}

		if(result) {
			/*Copy and store the iterator that points to the current object */
			iterList[numObjs++] = pgpCopyKeyIter (selectiter);
			if (notFirst) {
				/*XXX Ambiguous match if we're in batch mode...*/
				if (numObjs == 2) {
					InteractionOutputString(TRUE, "%s\n 1) ", header);
					objPrint(iterList[0], OUTPUT_INTERACTION);
				}
				InteractionOutputString(TRUE, "%2d) ", numObjs);
				objPrint (selectiter, OUTPUT_INTERACTION);
			}
			else
				notFirst = 1;
		}
	} while(result && (numObjs < maxObjs));

	if (numObjs < maxObjs) {
		if(numObjs < 1)
			result = 0;
		else {
			if(numObjs == 1) {
				objPrint(iterList[0], OUTPUT_INTERACTION);
/*				*iter = iterList[0];*/
				*iter = pgpCopyKeyIter (iterList[0]);
				pgpFreeKeyIter(selectiter);
				result = 1;
			}
			else {
				result = 0;
				while(!result) {
					InteractionOutput(TRUE, "CHOOSE_ONE_ABOVE");
					pgpTtyGetString(buf, sizeof(buf), TRUE, pgpEnv);
					choice = strtol(buf, &endNum, 10);
					while (isspace (*endNum))
						endNum++;
					if (choice >= 1 && choice <= numObjs) {
						*iter = pgpCopyKeyIter (iterList[choice - 1]);
						pgpFreeKeyIter (selectiter);
						result = numObjs;
					}
					else
						InteractionOutput(TRUE, "INVALID_SELECTION");
				}
			}
		}
	}
	else {
		ErrorOutput(TRUE, LEVEL_CRITICAL, "TOO_MANY_MATCHES");
		result = -1;
	}
		
	/*  Free all the key iterators we copied */
	for (i = 0; i < numObjs; i++)
		pgpFreeKeyIter (iterList[i]);

	return result;
}

#if 0

/*
 * Select one object of type <selecttype> and return an iterator pointing
 * to it.  The iterator passed is replaced by a new one.
 *
 * <targettype> defines the requirements for the returned object. For example,
 * if <selecttype> == key and <targettype> == sig, the selected key must
 * have a signed userid attached to it.
 */
static int
selectOneDBObj(PGPKeyIter **iter, int selecttype,
			   int targettype, char const *header)
{
	PGPKeyIter		*selectiter = *iter, *targetiter = NULL;
	PGPKeyIter		*iterList[50];
	PGPKey			*key = NULL;
	PGPUserID		*userid = NULL;
	PGPCert			*cert = NULL;
	int             maxObjs = sizeof(iterList) / sizeof(iterList[0]);
	int             numObjs = 0;
	int             notFirst = 0;
	int             result = 0;
	long            choice;
	char            *endNum, buf[32];
	int             i;

	for (;;) {
		result = 0;
		switch (selecttype) {
		case KDBTYPE_KEY:
			key = pgpKeyIterNext (selectiter);
			if (!key)
			    break;
			/*  Do we need a name or a sig on the key? */
			if (targettype == KDBTYPE_USERID || targettype == KDBTYPE_CERT) {
				targetiter = pgpCopyKeyIter (selectiter);
				for (;;) {
					userid = pgpKeyIterNextUserID (targetiter);
					if (userid && targettype == KDBTYPE_USERID) {
						result = 1;
						break;
					}
					else if (!userid)
						break;
					if (targettype == KDBTYPE_CERT) {
						cert = pgpKeyIterNextUIDCert (targetiter);
						if (cert) {
							result = 1;
							break;
						}
					}
				}
				pgpFreeKeyIter (targetiter);
			}
			else
				result = 1;
			break;
		case KDBTYPE_USERID:
			userid = pgpKeyIterNextUserID (selectiter);
			if (!userid)
			    break;
			if (targettype == KDBTYPE_CERT) {
				targetiter = pgpCopyKeyIter (selectiter);
				cert = pgpKeyIterNextUIDCert (targetiter);
				if (cert)
					result = 1;
				pgpFreeKeyIter (targetiter);
			}
			else
				result = 1;
			break;
		case KDBTYPE_CERT:
			cert = pgpKeyIterNextUIDCert (selectiter);
			if (cert)
				result = 1;
			break;
		}
		if (!result)
			break;

		if (numObjs >= maxObjs) {
			ErrorOutput(TRUE, LEVEL_CRITICAL, "TOO_MANY_MATCHES");
			goto destroyAndError;
		}

		/*  Copy and store the iterator that points to the current object */
		iterList[numObjs++] = pgpCopyKeyIter (selectiter);
		if (notFirst) {
			if (numObjs == 2) {
				InteractionOutputString(TRUE, "%s\n 1) ", header);
				objPrint (iterList[0], OUTPUT_INTERACTION);
			}
			InteractionOutputString(TRUE, "%2d) ", numObjs);
			objPrint (selectiter, OUTPUT_INTERACTION);
		}
		else
			notFirst = 1;
	}

	if (numObjs < 1)
		return 0;
	else if (numObjs == 1) {
		objPrint (iterList[0], OUTPUT_INTERACTION);
		*iter = iterList[0];
		pgpFreeKeyIter (selectiter);
		return numObjs;
	}
	InteractionOutput(TRUE, "CHOOSE_ONE_ABOVE");
	pgpTtyGetString(buf, sizeof(buf), TRUE, pgpEnv);
	choice = strtol(buf, &endNum, 10);
	while (isspace (*endNum))
		endNum++;
	if (*endNum || choice < 1 || choice > numObjs) {
		InteractionOutput(TRUE, "INVALID_SELECTION");
		goto destroyAndError;
	}
	*iter = pgpCopyKeyIter (iterList[choice - 1]);
	pgpFreeKeyIter (selectiter);
	result = numObjs;
	goto destroyAndExit;

destroyAndError:
	result = -1;
destroyAndExit:
	/*  Free all the key iterators we copied */
	for (i = 0; i < numObjs; i++)
		pgpFreeKeyIter (iterList[i]);
	return result;
}

#endif


/*
 * Service routine for selectDBObject, below.  This one selects a user ID
 * and optionally a signature from a RingSet composed of a single key
 * and its descendants.
 *	
 * <iter> points to a key object previously selected, and is updated to
 * point to the child object.
 */
static int
selectChildObject(PGPKeyIter **iter, 
				  int selecttype, 
				  char const *prep)
{
	int err = 0;
	char header[255], *string = NULL;;

	pgpAssert (selecttype == KDBTYPE_USERID || selecttype == KDBTYPE_CERT);

	if (selecttype == KDBTYPE_USERID)
		LoadString(&string, "PLEASE_SELECT_A_USER_ID");
	else
		LoadString(&string, "PLEASE_SELECT_A_USER_ID_WITH_SIG");

	if(string) {
		sprintf(header, string, prep);
		FreeString(&string);

		if((err = selectOneDBObj(iter,
								 KDBTYPE_USERID,
								 selecttype, header)) <= 0) {
			StatusOutput(TRUE, "NO_USER_IDS_SELECTED", prep);
			err = 0;
		}
		else {
			if (selecttype == KDBTYPE_CERT) {
				if((err = LoadString(&string, "PLEASE_SELECT_A_SIGNATURE")) ==
				   PGPERR_OK && string) {
					sprintf(header, string, prep);
					FreeString(&string);
					if((err = selectOneDBObj(iter,
											KDBTYPE_CERT,
											0,
											 header)) <= 0) {
						StatusOutput(TRUE, "NO_SIGNATURES_SELECTED", prep);
					}
				}
			}
		}
	}

	return err;	/* success */
}


/*
 * Selection routine. Selects a key, userid or
 * signature depending on the selecttype parameter, a RINGTYPE_ value.
 * Returns pointer to a key iterator that references the selected object.
 * Prep is a prepositional clause like "to be removed".  Returns 0 if
 * no keys selected, negative on error.  If objects are selected returns
 * the number of objects at that level, e.g. if a user ID is selected it
 * returns the number of user ID's on that key; if a signature is selected
 * it returns the number of sigs on that user ID.
 * This will always be at least 1.
 */
static int
selectDBObject(int argc, char **argv,
				 PGPKeySet *fullset, int selecttype, char const *prep,
			   PGPKeyIter **iter)
{
	PGPKeySet *subset = NULL;
	PGPKeyList *keylist = NULL;
	char namebuf[256];
	char header[256], *string = NULL;
	size_t namelen;
	int err = 0;
	int anykeys;
	extern PgpEnv *pgpEnv;

	/*
	 * Protect header[] from being overfilled by the sprintf's to it.
	 * prep is only set within the program to small strings, so this
	 * should never happen.
	 */
	pgpAssert (strlen(prep) < 100);

	if (argc == 0) {
		InteractionOutput(TRUE, "A_USERID_IS_REQUIRED", prep);
		namelen = pgpTtyGetString (namebuf, sizeof (namebuf), TRUE, pgpEnv);
		argv = (char **) malloc(sizeof(char *));
		argv[0] = namebuf;
		argc = 1;
	}

	/* Build ordered subset of keys */
	anykeys = selectKeyArgs (argc, argv, fullset, &subset, 0, FALSE);

	if (anykeys) {
		keylist = pgpOrderKeySet (subset, kPGPUserIDOrdering);
		if (keylist) {
			*iter = pgpNewKeyIter (keylist);
			if(*iter) {
				switch(selecttype) {
					case KDBTYPE_USERID:
						LoadString(&string,
								   "PLEASE_SELECT_A_KEY_WITH_USERID");
						break;

					case KDBTYPE_CERT:
						LoadString(&string, "PLEASE_SELECT_A_KEY_WITH_SIG");
						break;

					default:
						LoadString(&string, "PLEASE_SELECT_A_KEY");
						break;
				}

				if(string) {
					sprintf(header, string, prep);
					FreeString(&string);
					/*  Point the iterator at the selected key  */
					err = selectOneDBObj(iter,
										 KDBTYPE_KEY,
										 selecttype,
										 header);
				}
			}
			else {
				ErrorOutput(TRUE, LEVEL_CRITICAL, "UNABLE_TO_CREATE_ITER");
				err = PGPERR_NOMEM;
			}
		}
		else {
			ErrorOutput(TRUE, LEVEL_CRITICAL, "UNABLE_TO_ORDER_KEYSET");
			err = PGPERR_NOMEM;
		}
	}

	if (err <= 0) {
		StatusOutput(TRUE, "NO_KEYS_SELECTED", prep);
		err = 0;
	}
	else {
		/* Handle signature and user ID selection */
		if (selecttype != KDBTYPE_KEY) {
			err = selectChildObject(iter, selecttype, prep);
		}
	}

	if (keylist)
		pgpFreeKeyList (keylist);
 	if (subset)
		pgpFreeKeySet (subset);
	return err;
}

/*
 * Selects one key from those that match the given substring, or
 * from all keys if <subStr> is NULL.
 */
static PGPError
selectOneKey(char const *subStr, PGPKeySet *fullset, PGPKey **key,
			char const *header)
{
	PGPError		result;
	PGPKeySet *		matches;
	PGPKeyList *	matchList;
	PGPKeyIter *	iter = NULL;

	*key = NULL;

	if (subStr == NULL) {
		pgpIncKeySetRefCount(fullset);
		matches = fullset;
	} else {
		matches = pgpFilterKeySetAuto(fullset, subStr);
	}
	if (matches == NULL)
		return -1;
	
	matchList = pgpOrderKeySet(matches, kPGPUserIDOrdering);
	pgpFreeKeySet(matches);
	if (matchList == NULL)
		return PGPERR_NOMEM;

	iter = pgpNewKeyIter(matchList);
	pgpFreeKeyList(matchList);
	if (iter == NULL)
		return PGPERR_NOMEM;

	result = selectOneDBObj(&iter,
							KDBTYPE_KEY,
							KDBTYPE_KEY,
							(char *) header);
	if (result > 0)
	{
		result = PGPERR_OK;
		*key = pgpKeyIterKey(iter);
	}

	if(iter) {
		pgpFreeKeyIter(iter);
	}
	return result;
}


static int
doKeyAdd (int argc, char *argv[])
{
	PGPKeySet		*newset = NULL;
	PGPKeyList		*list = NULL;
	PGPFileRef		*fileRef = NULL;
	int             newkeys = 0;
	int             err = PGPERR_OK, port = 0;
	char server[1024], file[1024], protocol[16] = "\0";
	extern PgpEnv *pgpEnv;
	Boolean DoAdd = TRUE;

	if (!argc) {
		ErrorOutput(TRUE, LEVEL_CRITICAL, "NO_KEYFILE_SPECIFIED");
		err = PGPERR_MISSING_ARGUMENT;
	}
	
	while (argc--) {
		Boolean ImportedOK = FALSE;
		
		if((fileRef = pgpNewFileRefFromFullPath (argv[argc]))) {
			if((newset = pgpImportKeyFile (fileRef))) {
				ImportedOK = TRUE;
			}
			pgpFreeFileRef(fileRef);
		}
		
		if(!ImportedOK) {
			char *buffer = NULL;
			if(parse_url(protocol, server, file, &port, argv[argc]) != 
			   PGPERR_OK) {
				const char *pserver;

				protocol[0] = '\0';

				if(GetFileFromServer(argv[argc])) {
					/*Didn't look like an URL, let's try getting 
					  it as a userid*/
					strcpy(protocol, "hkp");
				
					pserver = pgpenvGetString(pgpEnv, 
											  PGPENV_HTTPKEYSERVERHOST, 
											  NULL, 
											  NULL);
					strcpy(server, pserver);
					strcpy(file, "/");
					strcat(file, argv[argc]);
					port = pgpenvGetInt(pgpEnv, 
										PGPENV_HTTPKEYSERVERPORT, 
										NULL, 
										NULL);
					StatusOutput(TRUE, 
								 "RETRIEVING_URL",
								 protocol,
								 server,
								 port,
								 file);
				}
			}
				
			if(protocol[0] && (GetKeyInfo(protocol, 
										  server, 
										  file, 
										  port, 
										  &buffer)  == PGPERR_OK)) {
				newset = pgpImportKeyBuffer((unsigned char *) buffer,
											strlen(buffer));
				ImportedOK = TRUE;
				pgpFree(buffer);
			}
			else {
				ErrorOutput(TRUE,
							LEVEL_SEVERE,
							"UNABLE_TO_IMPORT_KEYFILE",
							argv[argc]);
				continue;
			}
		}

		StatusOutputString(FALSE, "\n");

		if(newset) {
			StatusOutput(TRUE,
						 "ADDING_KEYS");
			/* Order keys for display */
			list = pgpOrderKeySet (newset, kPGPAnyOrdering);
			if (list) {
				kdbTtyKeyView(list, defaultset, argv[argc], 0);
				newkeys++;
				if(!pgpenvGetInt(pgpEnv, PGPENV_BATCHMODE, NULL, NULL)) {
					InteractionOutput(TRUE, "ADD_THESE_KEYS");
					DoAdd = pgpTtyGetBool (TRUE, TRUE, pgpEnv);
				}
				PrimaryOutputString("\n");
				if(DoAdd) {
					pgpAddKeys (defaultset, newset);
				}
				else {
					StatusOutput(TRUE, "ABORTED");
				}
				pgpFreeKeyList (list); list = NULL;
				/*
				 * Must commit changes before freeing source set, a
				 *  temporary library restriction.
				 */
				pgpCommitKeyRingChanges (defaultset);
				pgpFreeKeySet (newset); newset = NULL;
			}
			else {
				ErrorOutput(TRUE, LEVEL_CRITICAL, "UNABLE_TO_CREATE_KEYLIST");
				err = PGPERR_NOMEM;
			}
		}

		if (newkeys) {
			if(DoAdd) {
				StatusOutput(TRUE, "KEYS_ADDED_SUCCESSFULLY");
				err = PGPERR_OK;
			}
		}
		else {
			ErrorOutput(TRUE, LEVEL_CRITICAL, "NO_KEYS_TO_ADD");
		}
	}

	if (list)
		pgpFreeKeyList (list);
	if (newset)
		pgpFreeKeySet (newset);

	return err;
}

static int
doKeyExtract (struct Flags *flags, 
			  int argc,
			  char *argv[])
{
	PGPKeySet	*keyset = NULL;
	PGPFileRef	*fileRef = NULL;
	int err, port = 0;
	char server[1024], file[1024], protocol[16];
	extern PgpEnv *pgpEnv;

	err = selectKeyArgs (argc, argv, defaultset, &keyset, 0, TRUE);

	if(err > 0) {
		if(flags->outfile) {
			if(parse_url(protocol, server, file, &port, flags->outfile)
			   == PGPERR_OK) {
				/*No file specified, dump it out to the console:*/
				char *buffer = NULL;
				size_t length = 1;
			
				if((buffer = pgpAlloc(sizeof(char) * length))) {
					pgpExportKeyBuffer(keyset,
									   (unsigned char *) buffer,
									   &length);
					pgpFree(buffer);
					++length; /*Make room for a null*/
					if((buffer = pgpAlloc(sizeof(char) * length))) {
						err = pgpExportKeyBuffer(keyset,
												 (unsigned char *) buffer,
												 &length);
						if(err == PGPERR_OK) {
							if(strcasecmp(protocol, "hkp") == 0) {
								PutKeyInfoHkp(server, port, buffer);
							}
							else {
								if(strcasecmp(protocol, "http") == 0)
									ErrorOutput(TRUE,
												LEVEL_CRITICAL,
												"NO_HTTP_SEND");
								else
									ErrorOutput(TRUE,
												LEVEL_CRITICAL,
												"UNKNOWN_PROTOCOL",
												protocol);
							}
							pgpFree(buffer);
						}
					}
					else
						err = PGPERR_NOMEM;
				}
			}
			else {
				if(pgpTtyCheckOverwrite (pgpEnv, flags->outfile)) {
					err = PGPERR_NO_FILE;
				}
				else {
					fileRef =
						pgpNewFileRefFromFullPath ((char *) flags->outfile);
					if (fileRef != NULL) {
						err = pgpExportKeyFile (keyset, fileRef);
						pgpFreeFileRef (fileRef);
					}
					else
						err = PGPERR_NOMEM;
				}
			}
		}
		else {
			/*No file specified, dump it out to the console:*/
			char *buffer = NULL;
			size_t length = 1;
			
			if((buffer = pgpAlloc(sizeof(char) * length))) {
				pgpExportKeyBuffer(keyset,
								   (unsigned char *) buffer,
								   &length);
				pgpFree(buffer);
				++length; /*Make room for a null*/
				if((buffer = pgpAlloc(sizeof(char) * length))) {
					err = pgpExportKeyBuffer(keyset,
											 (unsigned char *) buffer,
											 &length);
					if(err == PGPERR_OK) {
						*(buffer + length) = '\0';
						PrimaryOutputString(buffer);
						pgpFree(buffer);
					}
				}
				else
					err = PGPERR_NOMEM;
			}
			else
				err = PGPERR_NOMEM;
		}
	}
	else {
		if(err == 0)
			ErrorOutput(TRUE,
						LEVEL_CRITICAL,
						"NO_KEYS_SELECTED_FOR_EXTRACTION");
	}

	if(keyset)
		pgpFreeKeySet (keyset);

	return err;
}

static int
doKeyDisable (int argc, char *argv[])
{
	Boolean disabled;
	int done_something = 0;
	int err;
	PGPKeyIter     *iter = NULL;
	PGPKey         *key;
	char *SentenceFragment = NULL;
	extern PgpEnv *pgpEnv;

	if(LoadString(&SentenceFragment, "TO_DISABLE_OR_ENABLE") == PGPERR_OK &&
	   SentenceFragment) {

		/* Select object to remove */
		if((err = selectDBObject(argc, argv, defaultset, KDBTYPE_KEY,
								 SentenceFragment, &iter))
		   > PGPERR_OK) {
			key = pgpKeyIterKey (iter);
			pgpFreeKeyIter (iter);
			pgpGetKeyBoolean (key, kPGPKeyPropIsDisabled, &disabled);

			InteractionOutputString(TRUE, "\n");
			if (disabled) {
				InteractionOutput(TRUE, "ENABLE_THIS_KEY");
				if (pgpTtyGetBool (0, TRUE, pgpEnv)) {
					if ((err = pgpEnableKey (key)) == PGPERR_OK) {
						done_something = 1;
						StatusOutput(TRUE, "KEY_ENABLED");
					}
					else
						PGPErrCodeOutput(TRUE, LEVEL_CRITICAL, err);
				}
			}
			else {
				InteractionOutput(TRUE, "DISABLE_THIS_KEY");
				if (pgpTtyGetBool (0, TRUE, pgpEnv)) {
					if ((err = pgpDisableKey (key)) == PGPERR_OK) {
						done_something = 1;
							StatusOutput(TRUE, "KEY_DISABLED");
					}
					else
					{
						if(err == PGPERR_BADPARAM)
						{
							ErrorOutput(TRUE, 
										LEVEL_CRITICAL,
										"CANNOT_DISABLE_AXIOMATIC_KEYS");
						}
						else
							PGPErrCodeOutput(TRUE, LEVEL_CRITICAL, err);
					}
				}
			}
			if (done_something)	
				pgpCommitKeyRingChanges (defaultset);
		}
		else
			err = PGPERR_OK;
	}
	return 0;
}

static int
doKeyCheck (struct Flags *flags, int argc, char *argv[])
{
	PGPKeySet  *subset = NULL;
	PGPKeyList *keylist = NULL;

	(void)flags;

	if (argc > 0) {
		selectKeyArgs (argc, argv, defaultset, &subset, 0, FALSE);
		if (subset == NULL)
			return -1;
		pgpCheckKeyRingCertificates (subset);
		pgpCommitKeyRingChanges (defaultset);
		keylist = pgpOrderKeySet (subset, kPGPUserIDOrdering);
		kdbTtyKeyView (keylist, defaultset, NULL, 4);
		pgpFreeKeyList (keylist);
		pgpFreeKeySet (subset);
	}
	else {
		pgpCheckKeyRingCertificates (defaultset);
		pgpCommitKeyRingChanges (defaultset);
		keylist = pgpOrderKeySet (defaultset, kPGPUserIDOrdering);
		kdbTtyKeyView (keylist, defaultset, NULL, 4);
		pgpFreeKeyList (keylist);
	}
	return 0;
}


static int
doKeyList (struct Flags *flags, int argc, char *argv[])
{
	PGPKeySet *subset = NULL;
	PGPKeyList *keylist = NULL;
	int list_mode = 0;

	selectKeyArgs (argc, argv, defaultset, &subset, 1, TRUE);
	if (subset == NULL)
		return -1;

	if (flags->argc > 0 && flags->args[0] == 'l')
	        list_mode = 2;            /* verbose */
	keylist = pgpOrderKeySet (subset, kPGPUserIDOrdering);
	kdbTtyKeyView (keylist, defaultset, NULL, list_mode);
	pgpFreeKeyList (keylist);
	pgpFreeKeySet (subset);
	return 0;
}

/*
 * Give the user an opportunity to edit the trust of object obj.
 * Return 1 if he changes it, 0 if he does not.
 * firsttime means that we have not previously had a trust value for
 * this key, which changes the wording of the questions somewhat.
 * If PGPTRUSTMODEL is 0, obj should be a key, else it should be a name.
 * The key and/or name should have been printed just before this routine
 * is called.
 */
static int
doKeyEditTrust(PGPKeyIter *iter, int firsttime)
{
	byte keytrust;
	long oldtrust;
	unsigned long confvalue;
	unsigned long trustcode;
	int len;
#if ONLY_TRUST_VALID_KEYS
	long valid;
#endif
	PgpTrustModel pgptrustmodel;
	char buf[8];		/* enough for 7 digit response */
	extern PgpEnv *pgpEnv;

#if ONLY_TRUST_VALID_KEYS
	pgpGetKeyNumber (pgpKeyIterKey (iter), kPGPKeyPropValidity, &valid);
	if (valid != PGP_VALIDITY_COMPLETE) {
		ErrorOutput(TRUE, LEVEL_CRITICAL, "CANNOT_TRUST_INVALID_KEYS");
		return 0;
	}
#endif

	if (firsttime) {
		InteractionOutput(TRUE, "DO_YOU_WISH_TO_CHANGE_INTRODUCER_RELIABITY");
		if (!pgpTtyGetBool(0, TRUE, pgpEnv)) {
			StatusOutput(TRUE, "NO_CHANGES_MADE");
			return 0;
		}
		InteractionOutput(TRUE, "DETERMINE_IN_YOUR_MIND");
	}

	pgptrustmodel = PGPTRUST0;
	if (pgptrustmodel == PGPTRUST0) {
		InteractionOutput(TRUE, "WOULD_YOU_TRUST_THIS_KEY_AND_OWNER");
		len = pgpTtyGetString (buf, 2, TRUE, pgpEnv);
		if (!len) {
			StatusOutput(TRUE, "NO_CHANGES_MADE");
			return 0;
		}
		trustcode = atoi(buf);
		switch (trustcode) {
		case 1:
			keytrust = PGP_KEYTRUST_UNKNOWN;
			break;
		case 2:
			keytrust = PGP_KEYTRUST_NEVER;
			break;
		case 3:
			keytrust = PGP_KEYTRUST_MARGINAL;
			break;
		case 4:
			keytrust = PGP_KEYTRUST_COMPLETE;
			break;
		default:
			StatusOutput(TRUE, "UNRECOGNIZED_RESPONSE");
			StatusOutput(TRUE, "NO_CHANGES_MADE");
			return 0;
		}	
		if (pgpSetKeyTrust (pgpKeyIterKey (iter), keytrust))
			ErrorOutput(TRUE, LEVEL_CRITICAL, "UNABLE_TO_SET_TRUST");
	} else {					/* New trust model */
		InteractionOutput(TRUE, "DESCRIBE_CONFIDENCE_AS_INTRODUCER");
		if (!firsttime) {
			pgpGetUserIDNumber (pgpKeyIterUserID (iter),
									kPGPUserIDPropConfidence, &oldtrust);
			if (oldtrust == PGP_TRUST_INFINITE)
				InteractionOutput(TRUE, "CURRENTLY_INFINITE_TRUST");
			else if (oldtrust == 0)
				InteractionOutput(TRUE, "CURRENTLY_ZERO_TRUST");
			else {
				int d = oldtrust-PGP_TRUST_DECADE-PGP_TRUST_OCTAVE;
				int i;
				unsigned long l;
				InteractionOutput(TRUE,
								  "CURRENTLY_HAS_PERCENT_TRUST_START");
				d -= d % PGP_TRUST_DECADE;
				i = d / PGP_TRUST_DECADE;
				l = ringTrustToInt(oldtrust - d);
				if (i)
					InteractionOutputString(FALSE, "%lu%0*u", l, i, 0);
				else
					InteractionOutputString(FALSE, "%lu", l);
				InteractionOutput(TRUE,
								  "CURRENTLY_HAS_PERCENT_TRUST_END");
			}
		}
		InteractionOutput(TRUE, "ENTER_A_TRUST_RANGE");
		if (!firsttime)
			InteractionOutput(FALSE, "OR_HIT_RETURN_TO_LEAVE_UNCHANGED");

		InteractionOutput(TRUE, "WILL_BE_WRONG_TIME_TIME_IN");
		len = pgpTtyGetString (buf, sizeof(buf), TRUE, pgpEnv);
		if (!len) {
			StatusOutput(TRUE, "NO_CHANGES_MADE");
			return 0;
		}
		trustcode = strtoul(buf, NULL, 0);
		confvalue = ringIntToTrust(trustcode);
		pgpSetUserIDConf(pgpKeyIterUserID (iter), confvalue);
	}
	pgpCommitKeyRingChanges (defaultset);
	return 1;		/* made a change */
}


/* Helper routine for doKeyEdit, when the key being edited is someone else's */
static int
doKeyEditOthers(struct Flags *flags, PGPKeyIter *iter)
{
	int err = PGPERR_OK;
	PgpTrustModel pgptrustmodel;
	Boolean secret, SetAxiomatic = FALSE;
	PGPKey *key = pgpKeyIterKey (iter);
	char *string = NULL;
	extern PgpEnv *pgpEnv;

	(void) flags;

	pgptrustmodel = PGPTRUST0;
	pgpGetKeyBoolean (key, kPGPKeyPropIsSecret, &secret);
	if (secret) {
		InteractionOutput(TRUE, "DO_YOU_WANT_THIS_KEY_AXIOMATIC");
		if (pgpTtyGetBool(0, TRUE, pgpEnv)) {
		    if((err = pgpSetKeyAxiomatic (key, FALSE, NULL)) == PGPERR_OK) {
				StatusOutput(TRUE, "PUBLIC_KEYRING_UPDATED");
				SetAxiomatic = TRUE;
				pgpCommitKeyRingChanges (defaultset);
			}
		}
	}

	if(!SetAxiomatic) {
		if (pgptrustmodel == PGPTRUST0) {
			if(doKeyEditTrust(iter, 0))
				err = PGPERR_OK;
		} else {
			/* New trust model works on a per user ID basis.*/
			/* Must select one. */
			if((err = LoadString(&string, "TO_EDIT")) == PGPERR_OK) {
				err = selectChildObject(&iter, KDBTYPE_USERID, string);
				FreeString(&string);
			
				kdbTtyShowKey(OUTPUT_INTERACTION, iter, defaultset, 0);
				err = doKeyEditTrust(iter, 0);
			}
		}
	}

	return(err);
}

/*
 * Helper routine for doKeyEditSelf, to change a pass phrase.
 * If parent is non-NULL, it is a subkey pass phrase.
 * secobj is the object getting its pass phrase changed, with seckey
 * having been made from it.
 */
static int
doKeyChangePassphrase(PGPKeyIter *iter)
{
	int         err = PGPERR_OK;
	char		oldphrase[PASSLEN], newphrase[PASSLEN], newphrase2[PASSLEN];
	PGPSubKey   *subkey;
	Boolean     EchoPhrase = FALSE;
	extern PgpEnv *pgpEnv;

	EchoPhrase = pgpenvGetInt(pgpEnv, PGPENV_SHOWPASS, NULL, NULL);

	InteractionOutput(TRUE, "NEED_OLD_PASSPHRASE");
	pgpTtyGetPass (EchoPhrase, oldphrase, PASSLEN, pgpEnv);
	InteractionOutput(TRUE, "NEED_NEW_PASSPHRASE");
	pgpTtyGetPass (EchoPhrase, newphrase, PASSLEN, pgpEnv);
	InteractionOutput(TRUE, "ENTER_IT_A_SECOND_TIME");
	pgpTtyGetPass (EchoPhrase, newphrase2, PASSLEN, pgpEnv);
	if (strcmp (newphrase, newphrase2) != 0) {
		ErrorOutput(TRUE, LEVEL_CRITICAL, "PASSPHRASES_ARE_DIFFERENT");
		return -1;
	}
	StatusOutput(TRUE, "CHANGING_MASTER_KEY_PASSPHRASE");
	err = pgpChangePassphrase (pgpKeyIterKey (iter), oldphrase, newphrase);
	if (err)
		StatusOutput(TRUE, "PASSPHRASE_CHANGE_FAILED_MASTER");
	else {
		while ((subkey = pgpKeyIterNextSubKey (iter)) != NULL) {
			StatusOutput(TRUE, "CHANGING_SUBKEY_PASSPHRASE");
			err = pgpChangeSubKeyPassphrase (subkey, oldphrase, newphrase);
			if (err) {
				StatusOutput(TRUE, "PASSPHRASE_CHANGE_FAILED_SUBKEY");
				break;
			}
		}
	}

	memset(newphrase, 0, sizeof(newphrase));
	memset(newphrase2, 0, sizeof(newphrase2));
	memset(oldphrase, 0, sizeof(oldphrase));

	if(err != PGPERR_OK) {
		PGPErrCodeOutput(TRUE, LEVEL_CRITICAL, err);
	}

	return err;
}


/* Helper routine for doKeyEdit, when the key being edited belongs to us */
static int
doKeyEditSelf(struct Flags *flags, PGPKeyIter *iter)
{
	int err = 0;
	char	passphrase[PASSLEN];
	PGPKey *key = pgpKeyIterKey (iter);
	extern PgpEnv *pgpEnv;

	(void)flags;		/* make the compiler happy */

	InteractionOutput(TRUE, "CONFIRM_NON_AXIOMATIC");
	if (pgpTtyGetBool (0, TRUE, pgpEnv)) {
	    pgpUnsetKeyAxiomatic (key);
		goto commit;
	}
	InteractionOutput(TRUE, "CONFIRM_ADD_NEW_USERID");
	if (pgpTtyGetBool (0, TRUE, pgpEnv)) {
		char namebuf[256];
		int namelen;

		InteractionOutput(TRUE, "ENTER_NEW_USERID");
		namelen = pgpTtyGetString (namebuf, sizeof (namebuf), TRUE, pgpEnv);
		if (!namelen) {
			ErrorOutput(TRUE, LEVEL_CRITICAL, "NO_NAME_ENTERED");
			goto cleanup;
		}

		pgpTtyGetPass (pgpenvGetInt(pgpEnv, PGPENV_SHOWPASS, NULL, NULL),
					   passphrase, 
					   PASSLEN, pgpEnv);
		err = pgpAddUserID(pgpKeyIterKey (iter),
						   namebuf,
						   namelen,
						   passphrase);
		if (err) {
			ErrorOutput(TRUE,
						LEVEL_CRITICAL,
						"UNABLE_TO_ADD_NEW_USERID",
						err);
			goto cleanup;
		}
#if 0
		/* debugging consistency check */
		{
			extern RingPool PGPKDBExport *pgpRingPool;
			int nsets, nfiles;
			ringPoolConsistent (pgpRingPool, &nsets, &nfiles);
			pgpAssert (nsets==5);
			pgpAssert (nfiles==3);
		}
#endif
	}


	InteractionOutput(TRUE, "CONFIRM_CHANGE_PASSPHRASE");
	if (pgpTtyGetBool(0, TRUE, pgpEnv)) {
		err = doKeyChangePassphrase(iter);
		if (err < 0) {
			err = PGPERR_OK; 
            /*doKeyChangePassphrase handles user notification*/
			goto cleanup;
		}
#if 0
		/* debugging consistency check */
		{
			extern RingPool PGPKDBExport *pgpRingPool;
			int nsets, nfiles;
			ringPoolConsistent (pgpRingPool, &nsets, &nfiles);
			pgpAssert (nsets==3);
			pgpAssert (nfiles==3);
		}
#endif
	}

	InteractionOutput(TRUE, "CONFIRM_SET_DEFAULT_KEY");
	if (pgpTtyGetBool (0, TRUE, pgpEnv))
	    pgpSetDefaultPrivateKey (pgpKeyIterKey (iter));

commit:
	StatusOutput(TRUE, "KEYRINGS_UPDATED");
	pgpCommitKeyRingChanges (defaultset);
cleanup:
	memset(passphrase, 0, sizeof(passphrase));
	return err;
}


static int
doKeyEdit (struct Flags *flags, int argc, char *argv[])
{
	char const *prep = "to edit";
	PGPKeyIter *iter = NULL;
	int err;
	Boolean axiomatic;

	(void)flags;		/* make the compiler happy */

	/* Select object to sign */
	err = selectDBObject (argc, argv, defaultset, KDBTYPE_KEY,
						  prep, &iter);
	if (err <= 0)
		return err;

	InteractionOutputString(FALSE, "\n");
	kdbKeyPrint (OUTPUT_INTERACTION, iter, 1);
	InteractionOutputString(FALSE, "\n");

	pgpGetKeyBoolean (pgpKeyIterKey (iter), kPGPKeyPropIsAxiomatic,
					  &axiomatic);
	if (axiomatic)
		err = doKeyEditSelf(flags, iter);
	else
	    err = doKeyEditOthers(flags, iter);
	pgpFreeKeyIter (iter);
	return err;
}


/*
 * This handles key, user ID (with -ru), or signature (with -rs) removal.
 */
static int
doKeyRemove (struct Flags *flags, int argc, char *argv[])
{
	int err = 0;
	PGPKeyIter	  *iter = NULL;
	PGPKeySet     *newset = NULL;
	PGPKey        *key = NULL;
	int removelevel;
	char *prep = NULL, *typename = NULL;
	Boolean IsSecret = FALSE, RemoveKey = FALSE;
	extern PgpEnv *pgpEnv;

	LoadString(&prep, "TO_BE_REMOVED_FRAGMENT");
	
	/* Distinguish what we are to remove */
	if (flags->argc && strchr(flags->args, 's')) {
		removelevel = KDBTYPE_CERT; /* signature */
		LoadString(&typename, "SIGNATURE_FRAGMENT");
	}
	else if (flags->argc && strchr(flags->args, 'u')) {
		removelevel = KDBTYPE_USERID; /* user id */
		LoadString(&typename, "USERID_FRAGMENT");
	}
	else {
		removelevel = KDBTYPE_KEY; /* key */
		LoadString(&typename, "KEY_FRAGMENT");
	}

	/* Select object to remove */
	InteractionOutputString(TRUE, "\n");

	err = selectDBObject(argc, argv, defaultset, removelevel,
							prep, &iter);
	if (err <= 0)
		goto cleanup;

	/* err holds number of possible objects at that level */
	if (removelevel == KDBTYPE_USERID && err == 1) {
		err = 0;
		ErrorOutput(TRUE,
					LEVEL_CRITICAL,
					"SELECTED_KEY_HAS_ONLY_ONE_USERID",
					prep);
		goto cleanup;
	}

	/* Confirm removal */
	StatusOutput(TRUE,
				 "FOLLOWING_OBJECT_HAS_BEEN_SELECTED",
				 typename,
				 prep);

	objPrint(iter, OUTPUT_STATUS);
	/* Perform removal, but don't write back yet */
	switch (removelevel) {
		case KDBTYPE_KEY:
			/*We only ask if they're trying to delete a secret key. UserIDs
			 *and certs can be gotten again from other sources, but you are
			 *presumably the only person with a copy of your secret key.
			 */
			key = pgpKeyIterKey(iter);
			pgpAssert(key);
			
			if(key) {
				pgpGetKeyBoolean(key, kPGPKeyPropIsSecret, &IsSecret);
				if(IsSecret) {
					InteractionOutput(TRUE,
									  "VERIFY_REMOVE_KEY_PUBLIC_PRIVATE");
					RemoveKey = pgpTtyGetBool(0, TRUE, pgpEnv);
				}
				else
					RemoveKey = TRUE;
				
				if(RemoveKey) {
					newset = pgpNewSingletonKeySet (pgpKeyIterKey(iter));
					err = pgpRemoveKeys (defaultset, newset);
					pgpFreeKeySet (newset);
					StatusOutput(TRUE, "REMOVED");
				}
				else {
					StatusOutput(TRUE, "CANCELED");
					err = PGPERR_OK;
				}
			}
			else {
				ErrorOutput(TRUE, LEVEL_CRITICAL, "UNABLE_TO_ITERATE_KEY");
				err = PGPERR_NOMEM;
			}
			break;
		case KDBTYPE_USERID:
			if((err = pgpRemoveUserID (pgpKeyIterUserID (iter))) == PGPERR_OK)
				StatusOutput(TRUE, "REMOVED");
			break;
		case KDBTYPE_CERT:
			if((err = pgpRemoveCert (pgpKeyIterCert (iter))) == PGPERR_OK)
				StatusOutput(TRUE, "REMOVED");
	}
	if (err) {
		ErrorOutput(TRUE, LEVEL_CRITICAL, "UNABLE_TO_REMOVE_OBJECT");
		goto cleanup;
	}

	/* Write out results */
	pgpCommitKeyRingChanges (defaultset);

	/* success */
	err = PGPERR_OK;

cleanup:
	if (iter)
	    pgpFreeKeyIter (iter);

	if(prep)
		FreeString(&prep);

	if(typename)
		FreeString(&typename);

	return err;
}


/*
 * Issue a signature on a userID.
 */
static int
doKeySign (struct Flags *flags, int argc, char *argv[])
{
	int err;
	char *prep = NULL;
	char passphrase[PASSLEN];
	PGPKeyIter  *iter = NULL;
	PGPKey *key;
	extern PgpEnv *pgpEnv;

	(void)flags;
	
	if (flags->signKey != NULL) {
		key = flags->signKey;
	} else {
		key = pgpGetDefaultPrivateKey (defaultset);
		if (!key) {
			ErrorOutput(TRUE, LEVEL_CRITICAL, "NO_DEFAULT_PRIVATE_KEY");
			return(PGPERR_NO_SECKEY);
		}
	}

	InteractionOutputString(TRUE, "\n");

	LoadString(&prep, "TO_BE_SIGNED_FRAGMENT");

	/* Select object to sign */
	err = selectDBObject(argc, argv, defaultset, KDBTYPE_USERID,
								prep, &iter);

	if(err > PGPERR_OK) {
		/* Confirm signature */
		InteractionOutput(TRUE, "VALIDITY_CERTIFICATION_WARNING");
		
		if (pgpTtyGetBool(0, TRUE, pgpEnv)) {
			StatusOutput(TRUE, "KEY_SELECTED_FOR_SIGNING_IS");
			objPrint (iter, OUTPUT_INTERACTION);
			pgpTtyGetPass (pgpenvGetInt(pgpEnv, PGPENV_SHOWPASS, NULL, NULL), 
						   passphrase, 
						   PASSLEN, pgpEnv);
			if((err = pgpCertifyUserID(pgpKeyIterUserID (iter),
									   key,
									   passphrase)) == PGPERR_OK) {
				StatusOutput(TRUE, "KEY_SIG_CERT_ADDED");
			
				pgpCommitKeyRingChanges (defaultset);
			}
			else {
				ErrorOutput(TRUE,
							LEVEL_CRITICAL,
							"KEY_SIGN_OPERATION_FAILED");
			}
		}
		else {
			StatusOutput(TRUE, "KEY_SIGNING_CANCELED");
		}
	}

	if(iter)
		pgpFreeKeyIter (iter);

	if(prep)
		FreeString(&prep);

	memset(passphrase, 0, sizeof(passphrase));
	return err;
}


/*
 * Revoke a key or a signature
 */
static int
doKeyRevoke (struct Flags *flags, int argc, char *argv[])
{
	int err;
	Boolean secret, revoked;
	PGPKeyIter  *iter = NULL;
	char *prep = NULL;
	int revokelevel;
	char passphrase[PASSLEN];
	extern PgpEnv *pgpEnv;

	LoadString(&prep, "TO_BE_REVOKED_FRAGMENT");

	/* Distinguish what we are to revoke */
	if (flags->argc && strchr(flags->args, 's'))
		revokelevel = KDBTYPE_CERT; /* signature on a name */
	else
		revokelevel = KDBTYPE_KEY; /* key */

	/* Select object to revoke */
	if((err = selectDBObject(argc, argv, defaultset, revokelevel,
							 prep, &iter)) > PGPERR_OK) {

		/* Display it for him */
		objPrint(iter, OUTPUT_INTERACTION);

		if (revokelevel == KDBTYPE_KEY) {
			/* Make sure it is our key */
			pgpGetKeyBoolean(pgpKeyIterKey (iter),
							 kPGPKeyPropIsSecret,
							 &secret);

			if (!secret) {
				ErrorOutput(TRUE,
							LEVEL_CRITICAL,
							"YOU_DONT_HAVE_THE_PRIVATE_KEY");
				err = PGPERR_NO_SECKEY;
				goto cleanup;
			}
			/* Make sure it hasn't already been revoked. */
			pgpGetKeyBoolean (pgpKeyIterKey (iter), kPGPKeyPropIsRevoked,
							  &revoked);
			if (revoked) {
				ErrorOutput(TRUE,
							LEVEL_CRITICAL,
							"KEY_IS_ALREADY_REVOKED");
				err = PGPERR_OK;
				goto cleanup;
			}
		}
		else {
			/* Make sure it hasn't already been revoked */
			pgpGetCertBoolean (pgpKeyIterCert (iter), kPGPCertPropIsRevoked,
							   &revoked);
			if (revoked) {
			    InteractionOutput(TRUE,
								  "SIG_ALREADY_REVOKED");
				if (!pgpTtyGetBool(0, TRUE, pgpEnv)) {
					StatusOutput(TRUE, "SIG_REVOCATION_CANCELLED");
					goto cleanup;
				}
			}
		}

		/* Confirm signature */
		if (revokelevel == KDBTYPE_KEY) {
			InteractionOutput(TRUE,
							  "CONFIRM_REVOKE_KEY");
		} else {
			InteractionOutput(TRUE,
							  "CONFIRM_REVOKE_SIG");
		}
		
		if (!pgpTtyGetBool(0, TRUE, pgpEnv)) {
			StatusOutput(TRUE, "REVOKE_CANCELLED");
			err = PGPERR_OK;
			goto cleanup;
		}
		
		pgpTtyGetPass (pgpenvGetInt(pgpEnv, PGPENV_SHOWPASS, NULL, NULL), 
					   passphrase, 
					   PASSLEN, pgpEnv);
		if (revokelevel == KDBTYPE_CERT)
			err = pgpRevokeCert(pgpKeyIterCert (iter),
								defaultset,
								passphrase);
		else
			err = pgpRevokeKey (pgpKeyIterKey (iter), passphrase);
		
		if (err)
			ErrorOutput(TRUE,
						LEVEL_CRITICAL,
						"UNABLE_TO_GENERATE_REVOCATION_SIGNATURE");
		else
			StatusOutput(TRUE, "KEY_REVOCATION_CERT_ADDED");
		pgpCommitKeyRingChanges (defaultset);
	}

cleanup:

	if(iter)
		pgpFreeKeyIter(iter);

	if(prep)
		FreeString(&prep);

	memset(passphrase, 0, sizeof(passphrase));

	return err;
}

static void
mainParseArgs (int *argcp, char *argv[], struct Flags *flags,
				struct PgpUICb *ui, struct PgpTtyUI *ui_arg)
{
	extern PgpEnv *pgpEnv;
	struct PgpOptContext opt;
	int c, argc = *argcp;

	pgpOptStart (&opt, argc, argv);
	argc = 0;

	while ((c = pgpOptNext (&opt)) != EOF) {
		switch (c) {
			case 0:
				if (opt.optarg[0] != '+' || opt.optarg[1] == '\0')
					argv[argc++] = opt.optarg;
				else if (pgpConfigLineProcess (ui, ui_arg, pgpEnv,
											   opt.optarg + 1,
											   PGPENV_PRI_CMDLINE)) {
					if(opt.optarg && *opt.optarg)
						ErrorOutput(TRUE,
									LEVEL_CRITICAL,
									"UNRECOGNIZED_OPTION_STRING",
									opt.optarg);
					exitUsage (PGPEXIT_ARGS);
				}
				break;

			case '-': {
				Boolean IsGoodArg = FALSE;
				/* This accepts --foo */
				/* -- is special-cased, so "-- foo" won't do. */

				/* Before we call the general case, we allow two --options
				 *in pgpk:  --revoke and --revokes.  So, we look for those,
				 *first.
				 */
				
				if(strncmp(opt.optarg, "revoke", strlen("revoke")) == 0 ||
					strcmp(opt.optarg, "version") == 0) {
					/*Is it a signature revocation?*/
					if(strcmp(opt.optarg, "revokes") == 0) {
						flags->opt = 'k';
						flags->args[flags->argc++] = 's';
						IsGoodArg = TRUE;
					}
					else {
						/*Make sure it's "revoke" and not "revokarooni" or
						 *something (smart alec!):
						 */
						if(strcmp(opt.optarg, "revoke") == 0) {
							flags->opt = 'k';
							IsGoodArg = TRUE;
						}
						else {
							if(strcmp(opt.optarg, "version") == 0) {
								exitVersion();
							}
						}
					}
				}
				else {
					/*It's not one of our args, so call the generic stuff
					 *that processess the config options, etc.
					 */
					IsGoodArg = !pgpConfigLineProcess(ui,
													  ui_arg,
													  pgpEnv,
													  opt.optarg,
													  PGPENV_PRI_CMDLINE);
				}
				if(!IsGoodArg) {
					ErrorOutput(TRUE,
								LEVEL_CRITICAL,
								"UNRECOGNIZED_OPTION_STRING_DASH",
								opt.optarg - 1);
					exitCleanup(PGPEXIT_ARGS);
				}
				opt.optarg = NULL;
				break;
			}

			case 'a':	/* Add */
			case 'c':	/* Check */
			case 'd':   /* Disable */
			case 'e':	/* Edit */
			case 'g':	/* Generate */
			case 'l':	/* List */
			case 'r':   /* Remove */
			case 's':	/* Sign */
			case 'x':	/* eXtract */
				setOpt (c, &opt, flags);
				break;
			case 'h':	/* Help */
				exitUsage (PGPEXIT_OK);
				/*NOTREACHED*/
			case 'o':	/* Output */
				if (flags->outfile) {
					ErrorOutput(TRUE, LEVEL_CRITICAL, "ONLY_ONE_OUTPUT_FILE");
					exitUsage(PGPEXIT_ARGS);
				}
				if (!opt.optarg) {
					ErrorOutput(TRUE, LEVEL_CRITICAL, "NO_OUTPUT_FILENAME");
					exitUsage(PGPEXIT_ARGS);
				}

				if(!(*opt.optarg == '-' && *(opt.optarg + 1) == '\0'))
					flags->outfile = opt.optarg;
				opt.optarg = 0;
				break;
				
			case 'u':	/* Username (myname) */
				if (flags->signName) {
					ErrorOutput(TRUE, LEVEL_CRITICAL, "ONLY_ONE_USERNAME");
					exitUsage(PGPEXIT_ARGS);
				}
				
				if (!opt.optarg) {
					ErrorOutput(TRUE, LEVEL_CRITICAL, "NO_USERID_SPECIFIED");
					exitUsage(PGPEXIT_ARGS);
				}
				
				flags->signName = opt.optarg;
				opt.optarg = 0;
				break;
			case 'K':	/* Keyring file */
				if (flags->ringfile) {
/*				exitArgError ("Only one -K option allowed");*/
					exitUsage(PGPEXIT_ARGS);
				}
				
				if (!opt.optarg) {
#if 0				
					exitArgError (
						"-K option requires a keyring filename argument");
#endif
					exitUsage(PGPEXIT_ARGS);
				}
				
				flags->ringfile = opt.optarg;
				opt.optarg = 0;
				break;
			case 'v':	/* Verbose */
				break;
			case 'q':   /* Quit */
				flags->quit = 1;
				break;
			default:
				if(c)
					ErrorOutput(TRUE,
								LEVEL_CRITICAL,
								"UNRECOGNIZED_OPTION_CHAR",
								c);
				exitUsage(PGPEXIT_OK);
		}
	}
    if(pgpenvGetInt(pgpEnv, PGPENV_LICENSE, NULL, NULL)) {
		display_license();
		exitCleanup (PGPEXIT_ARGS);
    }
	*argcp = argc;
	return;
}


static int
mainProcessFlags (struct Flags *flags, int argc,
		  char *argv[], PgpTtyUI *ui_arg)
{
	int ReturnCode = PGPERR_UNKNOWN_ARGUMENT;
	
	switch (flags->opt) {
		case 'a':
			ReturnCode = doKeyAdd(argc, argv);
			break;

		case 'c':
			ReturnCode = doKeyCheck (flags, argc, argv);
			break;

		case 'd':
			ReturnCode = doKeyDisable (argc, argv);
			break;

		case 'g':
			ReturnCode = doKeyGenerate (defaultset, argc, argv, ui_arg);
			break;

		case 'k':
			ReturnCode = doKeyRevoke (flags, argc, argv);
			break;

		case 'l':
			ReturnCode = doKeyList (flags, argc, argv);
			break;

		case 'r':
			ReturnCode = doKeyRemove (flags, argc, argv);
			break;

		case 's':
			ReturnCode = doKeySign (flags, argc, argv);
			break;

		case 'x':
			ReturnCode = doKeyExtract (flags, 
									   argc,
									   argv);
			break;

		case 'e':
			ReturnCode = doKeyEdit (flags, argc, argv);
			break;

		default:
			/*Dammit, I mean it!  Unknown Argument!  Now, stick!*/
			ReturnCode = PGPERR_UNKNOWN_ARGUMENT;
			break;
	}

	return(ReturnCode);
}

/* Clean up allocated space (like the pipeline and the rng) on exit */
static void
mainExit (void)
{
	if (defaultset) {
		pgpFreeKeySet (defaultset);
		defaultset = NULL;
	}
	return;
}

/* #define LOOP 1 to cause the program to loop and collect commands */

int
appMain (int argc, char *argv[])
{
	struct PgpUICb ui;
	struct PgpTtyUI ui_arg;
	struct Flags flags;
	int retval = 0;
	int nsets, nfiles;
	PGPError pgpErr = PGPERR_OK;
	extern RingPool *pgpRingPool;
	extern PgpEnv *pgpEnv;
	char *string = NULL;
	
	atexit(mainExit);

	pgpClearMemory(&ui, sizeof(ui));
	/* Setup the UI callback functions */
	ui.message = pgpTtyMessage;
	ui.doCommit = pgpTtyDoCommit;
	ui.newOutput = pgpTtyNewOutput;
	ui.needInput = pgpTtyNeedInput;
	ui.sigVerify = pgpTtySigVerify;
	ui.eskDecrypt = pgpTtyEskDecrypt;
	ui.annotate = NULL;

	/* Setup the TTY UI argument */
	ui_arg.verbose = 1;
	ui_arg.fp = stderr;
	ui_arg.ringset = NULL;
	ui_arg.showpass = 0;
	ui_arg.commits = -1;
	ui_arg.protect_name = 0;

	retval = pgpInitApp (&pgpEnv, &argc, &ui, &ui_arg, EXIT_PROG_PGPK, 0);

	ui_arg.env = pgpEnv;

	if (retval == PGPERR_OK) {
		/* Now initialize the values that need initialization */
/*		ui_arg.passcache = pgpPassCacheCreate (env);*/
		ui_arg.verbose = pgpenvGetInt(pgpEnv, PGPENV_VERBOSE, NULL, NULL);
		
		/* Check the exiration date on the application */
		exitExpiryCheck (pgpEnv);
	
		pgpLibInit();
		
		/* Setup the TTY UI argument */
#if 0
		pgpClearMemory(&ui_arg, sizeof(ui_arg));
#endif
		pgpClearMemory (&flags, sizeof (flags));
		mainParseArgs (&argc, argv, &flags, &ui, &ui_arg);

#if 0
		InitializeOutputHandlesToDefaults();
		LoadLanguageStrings(pgpEnv);
#endif
		
		enableUI = !pgpenvGetInt(pgpEnv, PGPENV_BATCHMODE, NULL, NULL);
		
		/* Open keyring file(s) */
		defaultset = NULL;
		if (flags.ringfile)
		{
			PGPFileRef *fileRef = pgpNewFileRefFromFullPath(flags.ringfile);
			
			if (fileRef == NULL)
				pgpErr = PGPERR_NO_FILE;
			else
			{
				defaultset = pgpOpenKeyRing(TRUE, 0, 1, fileRef, &pgpErr);
				pgpFreeFileRef(fileRef);
			}
		}
		else
			defaultset = pgpOpenDefaultKeyRings (TRUE, &pgpErr);
		if (defaultset) {
			/* Consistency check */
			ringPoolConsistent (pgpRingPool, &nsets, &nfiles);
			pgpAssert (nfiles <= 3);
			
			/*
			 * Set variables which may have been set on the command line.
			 */
			ui_arg.showpass = 0;
			if (flags.signName && !flags.signKey) {
				LoadString(&string, "SELECT_SIGNING_KEY");
				if (selectOneKey(flags.signName, defaultset, &flags.signKey,
								 string)
					!= PGPERR_OK ||
					flags.signKey == NULL) {
					/*XXX Should this use PGPEXIT_ARGS?*/
					ErrorOutput(TRUE,
								LEVEL_CRITICAL,
								"KEY_NOT_FOUND",
								flags.signName);
					FreeString(&string);
					exitCleanup (PGPEXIT_ARGS);
				}
				FreeString(&string);
			}
			
			if((retval = mainProcessFlags (&flags, argc, argv, &ui_arg))
			   != PGPERR_OK) {
				if (retval == PGPERR_UNKNOWN_ARGUMENT) {
					exitUsage(retval);
				}
				else {
					PGPErrCodeOutput(TRUE, LEVEL_CRITICAL, retval);
				}
				ringPoolConsistent (pgpRingPool, &nsets, &nfiles);
			}
			/* 2 sets per file, plus 1 for union; files includes memring */
			pgpAssert (nsets <= 5);
			pgpAssert (nfiles <= 3);
		}
		else {
			ErrorOutput(TRUE, LEVEL_CRITICAL, "UNABLE_TO_OPEN_KEYRING");
			PGPErrCodeOutput(TRUE, LEVEL_CRITICAL, pgpErr);
		}
	}
	else {
		ErrorOutput(TRUE, LEVEL_CRITICAL, "PGPINITAPP_FAILED");
		PGPErrCodeOutput(TRUE, LEVEL_CRITICAL, retval);
	}

	return retval;
}

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */

#if 0
#if 0				
				AddKeysResult = pgpAddKeys(TmpSet, SingleMatchSet);
#endif
			}
		}

#if 0
		if(AddKeysResult == PGPERR_OK && TmpSet) {
			*subset = pgpFilterKeySetAuto(fullset, argv[0]);
			pgpFreeKeySet(TmpSet);
			char foo[8096];
			int len;

			len = sizeof(foo);
			pgpExportKeyBuffer(TmpSet, foo, &len);
			*subset = pgpImportKeyBuffer(foo, strlen(foo));
			pgpIncKeySetRefCount(*subset);
			*subset = pgpFilterKeySetAuto(TmpSet, argv[0]);
			pgpIncKeySetRefCount(*subset);
			*subset = pgpCopyKeySet(TmpSet);
			*subset = pgpFilterKeySetAuto(fooset, argv[0]);
			pgpCountKeys(*subset);
			pgpFreeKeySet(*subset);
			AddKeysResult = PGPERR_OK;
		}
#endif
#endif

