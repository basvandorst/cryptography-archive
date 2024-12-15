/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1992, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

#include "af.h"

/*----------------------------------------------------------------------------*/
/*     C A  s                                                                 */
/*----------------------------------------------------------------------------*/
   
#define DEF_ISSUER_ALGID        md2WithRsaEncryption    /* default CA signature algid   */
#define DEF_SUBJECT_SIGNALGID   rsa           /* default CA encryption algid  */
#define DEF_SUBJECT_ENCRALGID   rsa           /* default CA encryption algid  */


#define CALOG		"calog"
#define	LOGFLAGS	"a+"

#define	LOGINIT		fprintf(logfile,"%s : INIT\n", logtime());
#define	LOGERR(msg)	fprintf(logfile,"%s : ERROR\t%s\n", logtime(), msg);
#define	LOGAFERR	fprintf(logfile,"%s : ERROR\t%s\n", logtime(), err_stack->e_text);
#define	LOGSECERR	fprintf(logfile,"%s : ERROR\t%s\n", logtime(), err_stack->e_text);
#define LOGUSER(N)	fprintf(logfile,"%s : NEW USER REGISTERED    <%s>\n", logtime(), N);
#define LOGCERT(C)	fprintf(logfile,"%s : CERTIFICATE ISSUED FOR <%s>\n                    Validity %s - %s\n",\
			logtime(), aux_DName2Name((C)->tbs->subject), \
                        aux_readable_UTCTime((C)->tbs->valid->notbefore), aux_readable_UTCTime((C)->tbs->valid->notafter));\
			fprintf(logfile,"                    SerialNo: ");\
			aux_fprint_Serialnumber(logfile, (C)->tbs->serialnumber);

#define LOGCERTSIGN(C)	fprintf(logfile,"%s : SIGN CERTIFICATE ISSUED FOR <%s>\n                    Validity %s - %s\n",\
			logtime(), aux_DName2Name((C)->tbs->subject), \
                        aux_readable_UTCTime((C)->tbs->valid->notbefore), aux_readable_UTCTime((C)->tbs->valid->notafter));\
			fprintf(logfile,"                    SerialNo: ");\
			aux_fprint_Serialnumber(logfile, (C)->tbs->serialnumber);

#define LOGCERTENCR(C)	fprintf(logfile,"%s : ENCR CERTIFICATE ISSUED FOR <%s>\n                    Validity %s - %s\n",\
			logtime(), aux_DName2Name((C)->tbs->subject), \
                        aux_readable_UTCTime((C)->tbs->valid->notbefore), aux_readable_UTCTime((C)->tbs->valid->notafter));\
			fprintf(logfile,"                    SerialNo: ");\
			aux_fprint_Serialnumber(logfile, (C)->tbs->serialnumber);

#define LOGPEMCRL(C)    fprintf(logfile,"%s : CRL ISSUED BY <%s>\n                    next update: %s\n",\
			logtime(), aux_DName2Name((C)->crl->tbs->issuer), aux_readable_UTCTime((C)->crl->tbs->nextUpdate));

char	*logtime();

# ifndef FILE
# include <stdio.h>
# endif
FILE    *logfile;

#define USERDB	 "user"
#define CERTDB	 "cert"
#define CRLDB    "crlwithcerts"
