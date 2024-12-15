#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
struct pgperr_t_to_str {
    int errornum;
    const char *errorname;
};

#define PGPERR(err, string) { err, #err },

static struct pgperr_t_to_str const ErrStringList[] = {
#include "pgpErr.h"
    { 0, 0 } /*End-of-list placeholder*/
};

#include "pgpTypes.h"
#include "pgpOutput.h"

int PGPErrCodeOutput(Boolean DisplayHeader, PgpSeverityLevel Level, int error)
{
    unsigned i, FoundCode = 0, CodeIndex = 0;
    int ReturnCode = PGPERR_OK;

    for(i = 0;
	i < sizeof(ErrStringList) / sizeof(*ErrStringList) && !FoundCode;
	++i) {
	if(ErrStringList[i].errornum == error) {
	    FoundCode = 1;
	    CodeIndex = i;
	}
    }

    if(FoundCode) {
	ReturnCode = ErrorOutput(DisplayHeader,
				 Level,
				 ErrStringList[CodeIndex].errorname);
    }
    else {
	ReturnCode = ErrorOutput(DisplayHeader,
				 Level,
				 "UNKNOWN_ERROR",
				 error);
    }
	
    return(ReturnCode);
}

int PGPErrCodeLoadString(char **String,
			 int error)
{
    unsigned i, FoundCode = 0, CodeIndex = 0;
    int ReturnCode = PGPERR_OK;

    for(i = 0;
	i < sizeof(ErrStringList) / sizeof(*ErrStringList) && !FoundCode;
	++i) {
	if(ErrStringList[i].errornum == error) {
	    FoundCode = 1;
	    CodeIndex = i;
	}
    }

    if(FoundCode) {
	ReturnCode = LoadString(String,
				ErrStringList[CodeIndex].errorname);
    }
    else {
	ReturnCode = LoadString(String,
				"UNKNOWN_ERROR");
    }
	
    return(ReturnCode);
}
