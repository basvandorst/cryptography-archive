/* ====================================================================
 * Copyright (c) 1999 Ben Laurie.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by Ben Laurie
 *    for use in the Apache-SSL HTTP server project."
 *
 * 4. The name "Apache-SSL Server" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Ben Laurie
 *    for use in the Apache-SSL HTTP server project."
 *
 * THIS SOFTWARE IS PROVIDED BY BEN LAURIE ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL BEN LAURIE OR
 * HIS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of patches to the Apache HTTP server interfacing it
 * to SSLeay.
 * For more information on Apache-SSL see http://www.apache-ssl.org.
 *
 * For more information on Apache see http://www.apache.org.
 *
 * For more information on SSLeay see http://www.psy.uq.oz.au/~ftp/Crypto/.
 *
 * Ben Laurie <ben@algroup.co.uk>.
 *
 */

#include "httpd.h"
#include "http_config.h"
#include "http_log.h"
#include "util_script.h"
#include <keynote.h>

MODULE_VAR_EXPORT module apache_ssl_keynote_module;

typedef struct
    {
    array_header *aszTrustedAssertions;
    array_header *aszIssuerTemplates;
    } KeyNoteDirConfig;

static void *CreateKeyNoteDirConfig(pool *p,char *dummy)
    {
    KeyNoteDirConfig *pConfig=ap_pcalloc(p,sizeof(KeyNoteDirConfig));

    pConfig->aszTrustedAssertions=
      ap_make_array(p,1,sizeof *pConfig->aszTrustedAssertions);
    pConfig->aszIssuerTemplates=
      ap_make_array(p,1,sizeof *pConfig->aszIssuerTemplates);

    return pConfig;
    }

#define MergeArray(member)	new->member=ap_append_arrays(p,base->member, \
							     add->member)

static void *MergeKeyNoteDirConfig(pool *p,void *basev,void *addv)
    {
    KeyNoteDirConfig *base=(KeyNoteDirConfig *)basev;
    KeyNoteDirConfig *add=(KeyNoteDirConfig *)addv;
    KeyNoteDirConfig *new=
      (KeyNoteDirConfig *)ap_palloc(p,sizeof(KeyNoteDirConfig));

    MergeArray(aszTrustedAssertions);
    MergeArray(aszIssuerTemplates);

    return new;
    }

static void SetKeyNoteEnvironment(int nSession,request_rec *r)
    {
    array_header *pArray;
    table_entry *pElts;
    int n;

    ap_add_common_vars(r);
    ap_add_cgi_vars(r);

    pArray=ap_table_elts(r->subprocess_env);
    pElts=(table_entry *)pArray->elts;

    for(n=0 ; n < pArray->nelts ; ++n)
	{
	if(!pElts[n].key)
	    continue;
printf("env: CGI_%s=%s\n",pElts[n].key,pElts[n].val);
	kn_add_action(nSession,ap_pstrcat(r->pool,"CGI_",pElts[n].key,NULL),
		      pElts[n].val,0);
	}
    kn_add_action(nSession,"app_domain","Apache-SSL",0);
    }

// Not a very efficient at this stage, but wtf, eh?
static char *Substitute(pool *p,const char *szSource,const char *szTag,
			const char *szValue)
    {
    const char *s,*t;
    char *szResult="";

    for(s=t=szSource ; (s=strstr(s,szTag)) ; s+=strlen(szTag),t=s)
	{
	char *tmp=ap_pstrndup(p,t,s-t);
	szResult=ap_pstrcat(p,szResult,tmp,szValue,NULL);
	}
    szResult=ap_pstrcat(p,szResult,t,NULL);

    return szResult;
    }

static int AddIssuerAssertions(pool *p,KeyNoteDirConfig *pConfig,int nSession,
			       request_rec *r)
    {
    int n;
    char **aszIssuerTemplates=(char **)pConfig->aszIssuerTemplates->elts;
    int nIssuerTemplates=pConfig->aszIssuerTemplates->nelts;
    table *t=r->subprocess_env;

    for(n=0 ; n < nIssuerTemplates ; ++n)
	{
	int nRet;
	char *szAssertion=Substitute(p,aszIssuerTemplates[n],"@@ISSUER@@",
				     ap_table_get(t,"SSL_CLIENT_I_DN"));
	szAssertion=Substitute(p,szAssertion,"@@SUBJECT@@",
			       ap_table_get(t,"SSL_CLIENT_DN"));
	nRet=kn_add_assertion(nSession,szAssertion,strlen(szAssertion),
				  ASSERT_FLAG_LOCAL);
printf("Adding issuer assertion: %s\n",szAssertion);
	if(keynote_errno)
	    {
	    ap_log_error(APLOG_MARK,APLOG_ERR|APLOG_NOERRNO,
			 r->connection->server,
			 "Can't add KeyNote assertion (keynote_errno=%d): %s",
			 keynote_errno,szAssertion);
	    return FORBIDDEN;
	    }
	}

    return OK;
    }

static int AddTrustedAssertions(KeyNoteDirConfig *pConfig,int nSession,
				request_rec *r)
    {
    int n;
    char **aszTrustedAssertions=(char **)pConfig->aszTrustedAssertions->elts;
    int nTrustedAssertions=pConfig->aszTrustedAssertions->nelts;

    for(n=0 ; n < nTrustedAssertions ; ++n)
	{
	int nRet=kn_add_assertion(nSession,aszTrustedAssertions[n],
				  strlen(aszTrustedAssertions[n]),
				  ASSERT_FLAG_LOCAL);
printf("Adding assertion: %s\n",aszTrustedAssertions[n]);
	if(keynote_errno)
	    {
	    ap_log_error(APLOG_MARK,APLOG_ERR|APLOG_NOERRNO,
			 r->connection->server,
			 "Can't add KeyNote assertion (keynote_errno=%d): %s",
			 keynote_errno,aszTrustedAssertions[n]);
	    return FORBIDDEN;
	    }
	}

    return OK;
    }

static int CheckKeyNoteAssertions(request_rec *r)
    {
    static BOOL bChecking;
    KeyNoteDirConfig *pConfig=(KeyNoteDirConfig *)
      ap_get_module_config(r->per_dir_config,&apache_ssl_keynote_module);
    // NB: this currently pointlessly eats randomness
    int nSession=kn_init();
    static const char *aszResults[]={"deny","permit"};
    int nResult;
    //    char *szAuthorizer=ap_pstrcat(r->pool,"x509-dn:",
    //				  r->connection->client->szClientX509,NULL);
    char *szAuthorizer=r->connection->client->szClientX509;
    int nRet=OK;

    if(bChecking)
	return OK;

    bChecking=TRUE;	// we can reenter because we call cgi_add_vars, which does a sub_req_lookup, which calls check_access

    SetKeyNoteEnvironment(nSession,r);

    keynote_errno=0;

    //    kn_add_authorizer(nSession,"dummy");	// for now...

printf("auth=\"%s\"\n",szAuthorizer);
    kn_add_authorizer(nSession,szAuthorizer);

    nRet=AddTrustedAssertions(pConfig,nSession,r);
    if(nRet != OK)
	goto done;

    nRet=AddIssuerAssertions(r->pool,pConfig,nSession,r);
    if(nRet != OK)
	goto done;

    nResult=kn_do_query(nSession,aszResults,2);
printf("result=%d/%d\n",nResult,keynote_errno);
    if(keynote_errno)
	{
	ap_log_error(APLOG_MARK,APLOG_ERR|APLOG_NOERRNO,
		     r->connection->server,
		     "Can't evaluate KeyNote query (keynote_errno=%d)",
		     keynote_errno);
	nRet=FORBIDDEN;
	}
    else if(nResult == 0)
	nRet=FORBIDDEN;
    else if(nResult == 1)
	nRet=OK;
    else
	{
	ap_log_error(APLOG_MARK,APLOG_ERR|APLOG_NOERRNO,
		     r->connection->server,
		     "Weird KeyNote result=%d",nResult);
	nRet=FORBIDDEN;
	}

done:
    kn_close(nSession);
    bChecking=FALSE;

    return nRet;
    }

static const char *push_dir_string_array_from_file(cmd_parms *cmd,
						   char *pStruct,
						   char *szFile)
    {
    int nFD;
    struct stat sb;
    //    KeyNoteDirConfig *pConfig=(KeyNoteDirConfig *)pStruct;
    int offset=(int)(long)cmd->info;
    char **pszAssertion=ap_push_array(*(array_header **)(pStruct+offset));

    szFile=ap_server_root_relative(cmd->pool,szFile);

    nFD=open(szFile,O_RDONLY);
    if(nFD < 0)
	return ap_psprintf(cmd->pool,"Can't open %s: errno=%d",szFile,errno);

    if(fstat(nFD,&sb) < 0)
	return ap_psprintf(cmd->pool,"Can't stat %s: errno=%d",szFile,errno);
    
    *pszAssertion=ap_pcalloc(cmd->pool,sb.st_size+1);
    if(read(nFD,*pszAssertion,sb.st_size) != sb.st_size)
	return ap_psprintf(cmd->pool,"Can't read %s: errno=%d",szFile,errno);
    (*pszAssertion)[sb.st_size]='\0';

    return NULL;
    }

static command_rec keynote_cmds[]=
    {
    { "SSLKeyNoteTrustedAssertion",push_dir_string_array_from_file,
      (void *)XtOffsetOf(KeyNoteDirConfig,aszTrustedAssertions),OR_FILEINFO,
      ITERATE,"Add a trusted KeyNote assertion" },
    { "SSLKeyNoteTrustedIssuerTemplate",push_dir_string_array_from_file,
      (void *)XtOffsetOf(KeyNoteDirConfig,aszIssuerTemplates),OR_FILEINFO,
      ITERATE,"Add a trusted KeyNote assertion template for delegating from the issuer" },
    { NULL }
    };

module MODULE_VAR_EXPORT apache_ssl_keynote_module =
    {
    STANDARD_MODULE_STUFF,
    NULL,			/* initializer */
    CreateKeyNoteDirConfig,	/* dir config creater */
    MergeKeyNoteDirConfig,	/* dir merger --- default is to override */
    NULL,			/* server config */
    NULL,			/* merge server config */
    keynote_cmds,		/* command table */
    NULL,			/* handlers */
    NULL,			/* filename translation */
    NULL,			/* check_user_id */
    NULL,			/* check auth */
    CheckKeyNoteAssertions,	/* check access */
    NULL,			/* type_checker */
    NULL,			/* fixups */
    NULL,			/* logger */
    };
