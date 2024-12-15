/*
 * pgpOutput.c -- Functions to allow comunications with the user.
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Written by:	Brett A. Thomas <quark@baz.com>, <bat@pgp.com>
 *
 * $Id: pgpOutput.c,v 1.1.2.1.2.4 1997/08/19 21:00:09 quark Exp $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "pgpErr.h"
#include "pgpDebug.h"
#include "pgpMem.h"

#include "pgpBlockPrefs.h"
#include "pgpEnv.h"
#include "pgpOutput.h"
#include "language.h"

char OutputHeader[OUTPUT_LAST + 1][4] =
{"PRI",
 "INF",
 "QRY",
 "STA",
 "WRN",
 "ERR",
 "\0"
};

Boolean gShowHeaders = FALSE;

static PREFS *pInternalLanguageStrings = NULL;
static FILE *OutputHandle[OUTPUT_LAST];
static int RawOutputID(int OutputType,
		       PgpSeverityLevel Level,
		       Boolean DisplayHeader,
		       const char *StringID,
		       const char *DisplayLanguage,
		       va_list ap);
static int RawOutputString(int OutputType,
			   PgpSeverityLevel Level,
			   Boolean DisplayHeader,
			   const char *String,
			   va_list ap);

const char *Language;

int LoadLanguageStrings(struct PgpEnv const *env)
{
    char *LanguageFile = NULL;
    int ReturnCode = PGPERR_OK;

    pgpAssert(!pInternalLanguageStrings);
    pgpAssert(env);

    if((LanguageFile = (char *) pgpenvGetString(env,
						PGPENV_LANGUAGEFILE,
						NULL,
						NULL))) {
	ReturnCode = pgpBlockPrefsLoadFile(&pInternalLanguageStrings,
					   LanguageFile);
    }

    if(!LanguageFile) {
	fprintf(stderr,
		"\
Unable to find language file in your configuration file.\n");
    }

    if(!LanguageFile || ReturnCode != PGPERR_OK) {
	/*Go with the internal strings*/
	ReturnCode = pgpBlockPrefsLoadBuffer(&pInternalLanguageStrings,
					     (char *) lang);
	
    }

    return(ReturnCode);
}

int FreeLanguageStrings(void)
{
    int ReturnCode = PGPERR_OK;

    if(pInternalLanguageStrings) {
	ReturnCode = pgpBlockPrefsFreePrefs(&pInternalLanguageStrings);
    }

    return(ReturnCode);
}

int InitializeOutputHandlesToDefaults(void)
{
    int ReturnCode = PGPERR_OK;

    if((ReturnCode = SetOutputHandle(OUTPUT_PRIMARY, stdout)) == PGPERR_OK &&
       (ReturnCode = SetOutputHandle(OUTPUT_INFORMATION, stderr))
         == PGPERR_OK &&
       (ReturnCode = SetOutputHandle(OUTPUT_INTERACTION, stderr))
        == PGPERR_OK &&
       (ReturnCode = SetOutputHandle(OUTPUT_WARNING, stderr)) == PGPERR_OK &&
       (ReturnCode = SetOutputHandle(OUTPUT_ERROR, stderr)) == PGPERR_OK &&
       (ReturnCode = SetOutputHandle(OUTPUT_STATUS, stderr))) {
	ReturnCode = SetOutputHandle(OUTPUT_LAST, NULL);
    }

    return(ReturnCode);
}

int InitializeOutputHandlesToConfigValues(PgpEnv *env)
{
    int ReturnCode = PGPERR_OK;

    if((ReturnCode = 
	SetOutputHandleFromConfig(env, OUTPUT_PRIMARY, 
				  PGPENV_OUTPUTPRIMARYFD)) == PGPERR_OK &&
       (ReturnCode = 
	SetOutputHandleFromConfig(env, OUTPUT_INFORMATION, 
				  PGPENV_OUTPUTINFORMATIONFD)) == PGPERR_OK &&
       (ReturnCode = 
	SetOutputHandleFromConfig(env, OUTPUT_INTERACTION, 
				  PGPENV_OUTPUTINTERACTIONFD)) == PGPERR_OK &&
       (ReturnCode = 
	SetOutputHandleFromConfig(env, OUTPUT_WARNING,
				  PGPENV_OUTPUTWARNINGFD)) == PGPERR_OK &&
       (ReturnCode = 
	SetOutputHandleFromConfig(env, OUTPUT_ERROR, 
				  PGPENV_OUTPUTERRORFD)) == PGPERR_OK &&
       (ReturnCode = 
	SetOutputHandleFromConfig(env, OUTPUT_STATUS,
				  PGPENV_OUTPUTSTATUSFD)) == PGPERR_OK) {
	ReturnCode = SetOutputHandle(OUTPUT_LAST, NULL);
	Language = pgpenvGetString(env, PGPENV_LANGUAGE, NULL, NULL);
    }

    return(ReturnCode);
}

int SetOutputHandleFromConfig(PgpEnv *env, 
			      PgpOutputType OutputType,
			      enum PgpEnvInts var)
{
    int ReturnCode = PGPERR_OK, FD;

    FD = pgpenvGetInt(env, var, NULL, &ReturnCode);
    if(ReturnCode == PGPERR_OK)
    {
	/*Let's handle the common special cases...*/
	if(FD == 1)
	{
	    SetOutputHandle(OutputType, stdout);
	}
	else
	{
	    if(FD == 2)
	    {
		SetOutputHandle(OutputType, stderr);
	    }
	    else
	    {
		FILE *handle;
		
		if((handle = fdopen(var, "w")))
		{
		    ReturnCode = SetOutputHandle(OutputType, handle);
		}
		else
		{
		    ReturnCode = PGPERR_NO_FILE;
		}
	    }
	}
    }
    return(ReturnCode);
}

int SetOutputHandle(PgpOutputType HandleType, FILE *FileHandle)
{
    int ReturnCode = PGPERR_OK;

    OutputHandle[HandleType] = FileHandle;

    return(ReturnCode);
}

void SetHeaders(Boolean ShowHeaders)
{
    gShowHeaders = ShowHeaders;
}

int PrimaryOutput(const char *StringID, ...)
{
    va_list ap;

    va_start(ap, StringID);
    return(RawOutputID(OUTPUT_PRIMARY,
		       0,
		       TRUE,
		       StringID,
		       Language,
		       ap));
    va_end(ap);
}

int InformationOutput(Boolean DisplayHeader, const char *StringID, ...)
{
    va_list ap;

    va_start(ap, StringID);
    return(RawOutputID(OUTPUT_INFORMATION,
		       0,
		       DisplayHeader,
		       StringID,
		       Language,
		       ap));
    va_end(ap);
}

int StatusOutput(Boolean DisplayHeader, const char *StringID, ...)
{
    va_list ap;

    va_start(ap, StringID);
    return(RawOutputID(OUTPUT_STATUS,
		       0,
		       DisplayHeader,
		       StringID,
		       Language,
		       ap));
    va_end(ap);
}

int InteractionOutput(Boolean DisplayHeader, const char *StringID, ...)
{
    va_list ap;

    va_start(ap, StringID);
    return(RawOutputID(OUTPUT_INTERACTION,
		       0,
		       DisplayHeader,
		       StringID,
		       Language,
		       ap));
    va_end(ap);
}

int WarningOutput(Boolean DisplayHeader,
		  PgpSeverityLevel Level,
		  const char *StringID, ...)
{
    va_list ap;

    va_start(ap, StringID);
    return(RawOutputID(OUTPUT_WARNING,
		       Level,
		       DisplayHeader,
		       StringID,
		       Language,
		       ap));
    va_end(ap);
}

int ErrorOutput(Boolean DisplayHeader,
		PgpSeverityLevel Level,
		const char *StringID, ...)
{
    va_list ap;

    va_start(ap, StringID);
    return(RawOutputID(OUTPUT_ERROR,
		       Level,
		       DisplayHeader,
		       StringID,
		       Language,
		       ap));
    va_end(ap);
}

static int RawOutputID(int OutputType,
		       PgpSeverityLevel Level,
		       Boolean DisplayHeader,
		       const char *StringID,
		       const char *DisplayLanguage,
		       va_list ap)
{
    char *OutputString;
    int ReturnCode = PGPERR_OK;

    pgpAssert(pInternalLanguageStrings);

    if((ReturnCode = pgpBlockPrefsGetValue(pInternalLanguageStrings,
					   StringID,
					   DisplayLanguage,
					   &OutputString)) == PGPERR_OK) {
	ReturnCode = RawOutputString(OutputType,
				     Level,
				     DisplayHeader,
				     OutputString,
				     ap);
	pgpBlockPrefsFreeValue(&OutputString);
    }
    else {
	switch(ReturnCode) {
	    case PGPERR_STRING_NOT_FOUND:
		fprintf(OutputHandle[OutputType],
			"Error!  Unable to load string %s.\n", StringID);
		break;

	    case PGPERR_STRING_NOT_IN_LANGUAGE: {
		if(strcasecmp(DisplayLanguage, "us") != 0) {
		    ReturnCode = RawOutputID(OutputType,
					     Level,
					     DisplayHeader,
					     StringID,
					     "us",
					     ap);
		}
		else {
		    fprintf(OutputHandle[OutputType],
			    "\
The string %s is not defined in the language %s.\n",
			    StringID,
			    Language);
		}
		break;
	    }
	}
    }

    return(ReturnCode);
}

static int RawOutputString(int OutputType,
			   PgpSeverityLevel Level,
			   Boolean DisplayHeader,
			   const char *String,
			   va_list ap)
{
    if(DisplayHeader && gShowHeaders)
	fprintf(OutputHandle[OutputType], "%s:  ", OutputHeader[OutputType]);
    vfprintf(OutputHandle[OutputType], String, ap);
    fflush(OutputHandle[OutputType]);

    return(PGPERR_OK);
}

int LoadString(char **String, const char *StringID)
{
    int ReturnCode;

    if((ReturnCode = pgpBlockPrefsGetValue(pInternalLanguageStrings,
					   StringID,
					   Language,
					   String)) != PGPERR_OK) {
	ReturnCode = pgpBlockPrefsGetValue(pInternalLanguageStrings,
					   StringID,
					   "us",
					   String);
    }

    return(ReturnCode);
}

int FreeString(char **String)
{
    return(pgpBlockPrefsFreeValue(String));
}


int InteractionOutputString(Boolean DisplayHeader,
			    const char *String, ...)
{
    va_list ap;

    va_start(ap, String);
    return(RawOutputString(OUTPUT_INTERACTION,
			   0,
			   DisplayHeader,
			   String,
			   ap));
    va_end(ap);
}

int InformationOutputString(Boolean DisplayHeader, const char *String, ...)
{
    va_list ap;

    va_start(ap, String);
    return(RawOutputString(OUTPUT_INFORMATION,
			   0,
			   DisplayHeader,
			   String,
			   ap));
    va_end(ap);
}

int StatusOutputString(Boolean DisplayHeader, const char *String, ...)
{
    va_list ap;

    va_start(ap, String);
    return(RawOutputString(OUTPUT_STATUS,
			   0,
			   DisplayHeader,
			   String,
			   ap));
    va_end(ap);
}

int WarningOutputString(Boolean DisplayHeader,
			PgpSeverityLevel Level,
			const char *String, ...)
{
    va_list ap;

    va_start(ap, String);
    return(RawOutputString(OUTPUT_WARNING,
			   Level,
			   DisplayHeader,
			   String,
			   ap));
    va_end(ap);
}

int ErrorOutputString(Boolean DisplayHeader,
		      PgpSeverityLevel Level,
		      const char *String, ...)
{
    va_list ap;

    va_start(ap, String);
    return(RawOutputString(OUTPUT_ERROR,
			   Level,
			   DisplayHeader,
			   String,
			   ap));
    va_end(ap);
}


int PrimaryOutputString(const char *String, ...)
{
    va_list ap;

    va_start(ap, String);
    return(RawOutputString(OUTPUT_PRIMARY,
			   0,
			   FALSE,
			   String,
			   ap));
    va_end(ap);
}

int SpecifiedOutput(Boolean DisplayHeader,
		    PgpOutputType OutputType,
		    PgpSeverityLevel Level,
		    const char *StringID, ...)
{
    va_list ap;

    va_start(ap, StringID);
    return(RawOutputID(OutputType,
		       Level,
		       DisplayHeader,
		       StringID,
		       Language,
		       ap));
    va_end(ap);
}

int SpecifiedOutputString(Boolean DisplayHeader,
			  PgpOutputType OutputType,
			  PgpSeverityLevel Level,
			  const char *String, ...)
{
    va_list ap;

    va_start(ap, String);
    return(RawOutputString(OutputType,
			   Level,
			   DisplayHeader,
			   String,
			   ap));
    va_end(ap);
}

int SpecifiedOutputBuffer(Boolean DisplayHeader,
			  PgpOutputType OutputType,
			  PgpSeverityLevel Level,
			  char *buffer,
			  size_t size,
			  size_t num)
{
    if(DisplayHeader && gShowHeaders)
	fprintf(OutputHandle[OutputType], "%s:  ", OutputHeader[OutputType]);

    fwrite(buffer, size, num, OutputHandle[OutputType]);
    fflush(OutputHandle[OutputType]);

    return(PGPERR_OK);
}





