/*
 * pgpOutput.h -- Functions to allow comunications with the user.
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Written by:	Brett A. Thomas <quark@baz.com>, <bat@pgp.com>
 *
 * $Id: pgpOutput.h,v 1.1.2.1.2.1 1997/07/08 12:59:26 quark Exp $
 */
typedef enum PgpOutputTypes {
    OUTPUT_PRIMARY = 0,
    OUTPUT_INFORMATION,
    OUTPUT_INTERACTION,
    OUTPUT_STATUS,
    OUTPUT_WARNING,
    OUTPUT_ERROR,
    OUTPUT_LAST
} PgpOutputType;

typedef enum PgpSeverityLevels {
    LEVEL_INFORMATION,
    LEVEL_SEVERE,
    LEVEL_CRITICAL
} PgpSeverityLevel;

int PrimaryOutput(const char *StringID, ...);
int InformationOutput(Boolean DisplayHeader,
		      const char *StringID, ...);
int InteractionOutput(Boolean DisplayHeader,
		      const char *StringID, ...);
int StatusOutput(Boolean DisplayHeader, const char *StringID, ...);
int WarningOutput(Boolean DisplayHeader,
		  PgpSeverityLevel Level,
		  const char *StringID, ...);
int ErrorOutput(Boolean DisplayHeader,
		PgpSeverityLevel Level,
		const char *StringID, ...);
int SpecifiedOutput(Boolean DisplayHeader,
		    PgpOutputType OutputType,
		    PgpSeverityLevel Level,
		    const char *StringID, ...);
int PGPErrCodeOutput(Boolean DisplayHeader,
		     PgpSeverityLevel Level,
		     int error);
void SetHeaders(Boolean ShowHeaders);

int InteractionOutputString(Boolean DisplayHeader, const char *String, ...);
int InformationOutputString(Boolean DisplayHeader, const char *String, ...);
int PrimaryOutputString(const char *String, ...);
int StatusOutputString(Boolean DisplayHeader, const char *String, ...);
int WarningOutputString(Boolean DisplayHeader,
			PgpSeverityLevel Level,
			const char *String, ...);
int ErrorOutputString(Boolean DisplayHeader,
			PgpSeverityLevel Level,
			const char *String, ...);
int SpecifiedOutputString(Boolean DisplayHeader,
			  PgpOutputType OutputType,
			  PgpSeverityLevel Level,
			  const char *String, ...);
int SpecifiedOutputBuffer(Boolean DisplayHeader,
			  PgpOutputType OutputType,
			  PgpSeverityLevel Level,
			  char *buffer,
			  size_t size,
			  size_t num);
int SetOutputHandle(PgpOutputType HandleType, FILE *FileHandle);
int InitializeOutputHandlesToDefaults(void);
int FreeLanguageStrings(void);
int LoadLanguageStrings(struct PgpEnv const *env);

int PGPErrCodeLoadString(char **String,
			 int error);
int LoadString(char **String, const char *StringID);
int FreeString(char **String);
