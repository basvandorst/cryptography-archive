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

/*
	CA utilities

	af_cadb_add_Certificate, af_cadb_get_Certificate, 
	af_cadb_add_user, af_cadb_get_user, af_cadb_add_CRLWithCertificates, af_cadb_list_CRLWithCertificates,
	af_cadb_get_CRLWithCertificates
	log CA events
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
/* #include <time.h> */
#include <errno.h>
#ifdef NDBM
#include <ndbm.h>
#else
#undef NULL
#include <dbm.h>
#endif

#include "cadb.h"



#ifdef __STDC__

static char *conv_to_upper	(char *c);

#else

static char *conv_to_upper();

#endif


/***************************************************************
 *
 * Procedure af_cadb_add_user
 *
 ***************************************************************/
#ifdef __STDC__

int af_cadb_add_user(
	Name	 *name,
	char	 *cadir
)

#else

int af_cadb_add_user(
	name,
	cadir
)
Name	 *name;
char	 *cadir;

#endif

{
#ifdef NDBM
	DBM	 * user;
#endif
	datum	key, data;
	int	i, rc;
	DName   *dname;
	Name    *cname;
	char    *userdbpath;
	char    *homedir;
	char	*proc = "af_cadb_add_user";

	if (!cadir) {
		aux_add_error(EINVALID, "Can't add user to databse because no CA directory specified", CNULL, 0, proc);
		return(-1);
	}

	if(cadir[0] != PATH_SEPARATION_CHAR) {
		homedir = getenv("HOME");
		userdbpath = (char *)malloc(strlen(homedir)+strlen(cadir)+strlen(USERDB)+3);
		if (!userdbpath) {
			aux_add_error(EMALLOC, "userdbpath", CNULL, 0, proc);
			return (-1);
		}
		strcpy(userdbpath, homedir);
		strcat(userdbpath, PATH_SEPARATION_STRING);
		strcat(userdbpath, cadir);
	}
	else {
		userdbpath = (char *)malloc(strlen(cadir)+strlen(USERDB)+2);
		if (!userdbpath) {
			aux_add_error(EMALLOC, "userdbpath", CNULL, 0, proc);
			return (-1);
		}
		strcpy(userdbpath, cadir);
	}
	if (userdbpath[strlen(userdbpath) - 1] != PATH_SEPARATION_CHAR) strcat(userdbpath, PATH_SEPARATION_STRING);
	strcat(userdbpath, USERDB);

	/*	store subject user record */
#ifdef NDBM
	user = dbm_open(userdbpath, O_RDWR, 0);
	if (!user) {
		aux_add_error(EINVALID, "Can't open user database", CNULL, 0, proc);
		free(userdbpath);
		return(-1);
	}
	free(userdbpath);
#else
	 {
		FILE * fd;
		char	fn[64];
		strcpy(fn, userdbpath);
		strcat(fn, ".pag");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		strcpy(fn, userdbpath);
		strcat(fn, ".dir");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		if (dbminit(userdbpath) < 0)  {
			free(userdbpath);
 			aux_add_error(EINVALID, "Can't open user database", CNULL, 0, proc);
			return(-1);
		}
		free(userdbpath);


	}
#endif
	dname = aux_Name2DName(name);
	if(!dname) {
		aux_add_error(EINVALID, "Can't add user to database because name is wrong", CNULL, 0, proc);
		return(-1);
	}		
	cname = aux_DName2CAPITALName(dname);
	aux_free_DName(&dname);
	key.dptr = cname;
	key.dsize = strlen(key.dptr);
	data.dptr = (char *)0;
	data.dsize = 0;
#ifdef NDBM
	rc = dbm_store(user, key, data, DBM_INSERT);
	if (rc < 0) {
		aux_add_error(EINVALID, "Can't store data to user database", CNULL, 0, proc);
		free(cname);
		return(rc);
	}

#else
	rc = store(key, data);
	if(rc < 0){
		aux_add_error(EINVALID, "Can't store data to user database", CNULL, 0, proc);
		free(cname);
		return(rc);
	}

#endif
	free(cname);
	if(rc == 0) LOGUSER(name);

#ifdef NDBM
	dbm_close(user);
#else
	dbmclose();
#endif

	return(0);
}


/***************************************************************
 *
 * Procedure af_cadb_get_user
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_IssuedCertificate *af_cadb_get_user(
	Name	 *name,
	char	 *cadir
)

#else

SET_OF_IssuedCertificate *af_cadb_get_user(
	name,
	cadir
)
Name	 *name;
char	 *cadir;

#endif

{
	SET_OF_IssuedCertificate *isscertset;
	OctetString 		 *ostr;
	char			 *userdbpath;
	char   			 *homedir;
	DName                    *dname;
	Name                     *cname;
	datum			  key, data;

	char			 *proc = "af_cadb_get_user";

#ifdef NDBM
	DBM	 * user;
#endif


	if (!cadir) {
		aux_add_error(EINVALID, "Can't get user from database because no CA directory specified", CNULL, 0, proc);
		return ((SET_OF_IssuedCertificate *)0);
	}

	if(cadir[0] != PATH_SEPARATION_CHAR) {
		homedir = getenv("HOME");
		userdbpath = (char *)malloc(strlen(homedir)+strlen(cadir)+strlen(USERDB)+3);
		if (!userdbpath) {
			aux_add_error(EMALLOC, "userdbpath", CNULL, 0, proc);
			return ((SET_OF_IssuedCertificate *)0);
		}
		strcpy(userdbpath, homedir);
		strcat(userdbpath, PATH_SEPARATION_STRING);
		strcat(userdbpath, cadir);
	}
	else {
		userdbpath = (char *)malloc(strlen(cadir)+strlen(USERDB)+2);
		if (!userdbpath) {
			aux_add_error(EMALLOC, "userdbpath", CNULL, 0, proc);
			return ((SET_OF_IssuedCertificate *)0);
		}
		strcpy(userdbpath, cadir);
	}
	if (userdbpath[strlen(userdbpath) - 1] != PATH_SEPARATION_CHAR) strcat(userdbpath, PATH_SEPARATION_STRING);
	strcat(userdbpath, USERDB);


	/*	fetch subject user record */
#ifdef NDBM
	if (!name) {
		aux_add_error(EINVALID, "no name given", CNULL, 0, proc);
		free(userdbpath);
		return ((SET_OF_IssuedCertificate *)0);
	}
	user = dbm_open(userdbpath, O_RDONLY, 0);
	free(userdbpath);
	if (!user) {
		aux_add_error(EINVALID, "Can't open user database", CNULL, 0, proc);
		return ((SET_OF_IssuedCertificate *)0);
	}
#else
	 {
		FILE * fd;
		char	fn[64];
		strcpy(fn, userdbpath);
		strcat(fn, ".pag");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		strcpy(fn, userdbpath);
		strcat(fn, ".dir");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		if (dbminit(userdbpath) < 0) {
			aux_add_error(EINVALID, "Can't open user database", CNULL, 0, proc);
			return ((SET_OF_IssuedCertificate *)0);
		}
	}
#endif
	dname = aux_Name2DName(name);
	if(!dname) {
		aux_add_error(EINVALID, "Can't get user from database because name is wrong", CNULL, 0, proc);
		return((SET_OF_IssuedCertificate *)0);
	}		
	cname = aux_DName2CAPITALName(dname);
	aux_free_DName(&dname);
	key.dptr = cname;
	key.dsize = strlen(key.dptr);
#ifdef NDBM
	data = dbm_fetch(user, key);
#else
	data = fetch(key);
#endif
	if (!data.dptr)	{
		aux_add_error(EINVALID, "Can't get data from user database", CNULL, 0, proc);
		free(cname);
		return ((SET_OF_IssuedCertificate *)0);
	}
	ostr = (OctetString *) malloc(sizeof(OctetString));
	if( !ostr ) {
		aux_add_error(EMALLOC, "ostr", CNULL, 0, proc);
		return ((SET_OF_IssuedCertificate *)0);
	}
	free(cname);
	ostr->noctets = data.dsize;
	ostr->octets = data.dptr;
	isscertset = d_SET_OF_IssuedCertificate(ostr);
	free(ostr);
#ifdef NDBM
	dbm_close(user);
#else
	dbmclose();
#endif

	return (isscertset);
}


/***************************************************************
 *
 * Procedure af_cadb_add_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

int af_cadb_add_Certificate(
	KeyType		  keytype,
	Certificate	 *newcert,
	char		 *cadir
)

#else

int af_cadb_add_Certificate(
	keytype,
	newcert,
	cadir
)
KeyType		  keytype;
Certificate	 *newcert;
char		 *cadir;

#endif

{
#ifdef NDBM
	DBM	 * user;
	DBM	 * cert;
#endif
	datum			   key, data;
	OctetString 		 * ostr;
	SET_OF_IssuedCertificate * new_isscertset, * old_isscertset;
	int			   i, * p, rc;
	char   			 * dbpath, * userdbpath, * certdbpath;
	char   			 * homedir, * username;

	char   			 * proc = "af_cadb_add_Certificate";


	if (!cadir) {
		aux_add_error(EINVALID, "Can't add certificate to database because no CA directory specified", CNULL, 0, proc);
		return (-1);
	}

	if(cadir[0] != PATH_SEPARATION_CHAR) {
		homedir = getenv("HOME");
		dbpath = (char *)malloc(strlen(homedir)+strlen(cadir)+3);
		if (!dbpath) {
			aux_add_error(EMALLOC, "dbpath", CNULL, 0, proc);
			return (-1);
		}
		strcpy(dbpath, homedir);
		strcat(dbpath, PATH_SEPARATION_STRING);
		strcat(dbpath, cadir);
	}
	else {
		dbpath = (char *)malloc(strlen(cadir)+2);
		if (!dbpath) {
			aux_add_error(EMALLOC, "dbpath", CNULL, 0, proc);
			return (-1);
		}
		strcpy(dbpath, cadir);
	}
	if (dbpath[strlen(dbpath) - 1] != PATH_SEPARATION_CHAR) strcat(dbpath, PATH_SEPARATION_STRING);

	userdbpath = (char *)malloc(strlen(dbpath)+strlen(USERDB)+1);
	strcpy(userdbpath, dbpath);
	strcat(userdbpath, USERDB);
	certdbpath = (char *)malloc(strlen(dbpath)+strlen(CERTDB)+1);
	strcpy(certdbpath, dbpath);
	strcat(certdbpath, CERTDB);
	free(dbpath);

	/*	store new cert entry */
#ifdef NDBM
	cert = dbm_open(certdbpath, O_RDWR, 0);
	if (!cert) 	{
		aux_add_error(EINVALID, "Can't open certificate database", CNULL, 0, proc);
		free(userdbpath);
		free(certdbpath);
		return(1);
	}
#else
	 {
		FILE * fd;
		char	fn[64];
		strcpy(fn, certdbpath);
		strcat(fn, ".pag");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		strcpy(fn, certdbpath);
		strcat(fn, ".dir");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		if (dbminit(certdbpath) < 0) {
			aux_add_error(EINVALID, "Can't open certificate database", CNULL, 0, proc);
			free(userdbpath);
			free(certdbpath);
			return(1);
		}

	}
#endif
	ostr = e_Certificate(newcert);
	if (!ostr)  {
		aux_add_error(EENCODE, "Can't encode certificate found in database", CNULL, 0, proc);
		free(userdbpath);
		free(certdbpath);
		return(1);
	}


	key.dptr = newcert->tbs->serialnumber->octets;
	key.dsize = newcert->tbs->serialnumber->noctets;
	data.dptr = ostr->octets;
	data.dsize = ostr->noctets;
#ifdef NDBM
	rc = dbm_store(cert, key, data, DBM_INSERT);
	if(rc < 0){
		aux_free_OctetString(&ostr);
		aux_add_error(EINVALID, "Can't store data to certifcate database", CNULL, 0, proc);
		free(userdbpath);
		free(certdbpath);
		return(1);
	}
#else
	if (store(key, data) < 0) {
		aux_free_OctetString(&ostr);
		aux_add_error(EINVALID, "Can't store data to certifcate database", CNULL, 0, proc);
		free(userdbpath);
		free(certdbpath);
		return(1);
	}
#endif
	if(keytype == SIGNATURE) {
		LOGCERTSIGN(newcert);
	}
	else if(keytype == ENCRYPTION) {
		LOGCERTENCR(newcert);
	}
	else {
		LOGCERT(newcert);
	}
#ifdef NDBM
	dbm_close(cert);
#else
	dbmclose();
#endif
	free(certdbpath);
	aux_free_OctetString(&ostr);

	/*	fetch subject user record */
#ifdef NDBM
	user = dbm_open(userdbpath, O_RDWR, 0);
	if (!user) {
		aux_add_error(EINVALID, "Can't open user database", CNULL, 0, proc);
		free(userdbpath);
		return(1);
	}

#else
	 {
		FILE * fd;
		char	fn[64];
		strcpy(fn, userdbpath);
		strcat(fn, ".pag");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		strcpy(fn, userdbpath);
		strcat(fn, ".dir");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		if (dbminit(userdbpath) < 0) {
			aux_add_error(EINVALID, "Can't open user database", CNULL, 0, proc);
			free(userdbpath);
			return(1);
		}

	}
#endif

	new_isscertset = (SET_OF_IssuedCertificate *) malloc(sizeof(SET_OF_IssuedCertificate));
	if( !new_isscertset ) {
		aux_add_error(EMALLOC, "new_isscertset", CNULL, 0, proc);
		free(userdbpath);
		return (1);
	}
	new_isscertset->element = (IssuedCertificate *) malloc(sizeof(IssuedCertificate));
	if( !new_isscertset->element ) {
		aux_add_error(EMALLOC, "new_isscertset->element", CNULL, 0, proc);
		free(userdbpath);
		return (1);
	}

	new_isscertset->element->serial = aux_cpy_OctetString(newcert->tbs->serialnumber);
	new_isscertset->element->date_of_issue = aux_current_UTCTime();

        username = (char *)aux_DName2Name(newcert->tbs->subject);
	old_isscertset = af_cadb_get_user(username, cadir);

	if (!old_isscertset)   /* first entry for subject */
		new_isscertset->next = (SET_OF_IssuedCertificate *)0;	
	else 
		new_isscertset->next = old_isscertset;

	ostr = e_SET_OF_IssuedCertificate(new_isscertset);

	key.dptr = conv_to_upper(username);
	key.dsize = strlen(key.dptr);
	data.dptr = ostr->octets;
	data.dsize = ostr->noctets;

#ifdef NDBM
	rc = dbm_store(user, key, data, DBM_REPLACE);
	if(rc < 0){
		aux_add_error(EINVALID, "Can't store data to user database", CNULL, 0, proc);
	        free(username);
	        aux_free_OctetString(&ostr);
		free(new_isscertset->element->date_of_issue);
		free(new_isscertset->element);
		free(new_isscertset);
		return(rc);
	}

	dbm_close(user);
#else
	rc = store(key, data);
	if(rc < 0){
		aux_add_error(EINVALID, "Can't store data to user database", CNULL, 0, proc);
	        free(username);
	        aux_free_OctetString(&ostr);
		free(new_isscertset->element->date_of_issue);
		free(new_isscertset->element);
		free(new_isscertset);
		return(rc);
	}
	dbmclose();
#endif
        free(username);
        aux_free_OctetString(&ostr);
	free(new_isscertset->element->date_of_issue);
	free(new_isscertset->element);
	free(new_isscertset);

	return(0);
}


/***************************************************************
 *
 * Procedure af_cadb_get_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

Certificate *af_cadb_get_Certificate(
	OctetString *serial,
	char	    *cadir
)

#else

Certificate *af_cadb_get_Certificate(
	serial,
	cadir
)
OctetString *serial;
char	    *cadir;

#endif

{
#ifdef NDBM
	DBM	    * cert;
#endif
	datum	      key, data;
	Certificate * certificate;
	OctetString * ostr;
	char        * certdbpath;
	char        * homedir;

	char   	    * proc = "af_cadb_get_Certificate";

	if (!cadir) {
		aux_add_error(EINVALID, "Can't get certificate from database because no CA directory specified", CNULL, 0, proc);
		return ((Certificate *)0);
	}

	if(cadir[0] != PATH_SEPARATION_CHAR) {
		homedir = getenv("HOME");
		certdbpath = (char *)malloc(strlen(homedir)+strlen(cadir)+strlen(CERTDB)+3);
		if (!certdbpath) {
			aux_add_error(EMALLOC, "certdbpath", CNULL, 0, proc);
			return ((Certificate *)0);
		}
		strcpy(certdbpath, homedir);
		strcat(certdbpath, PATH_SEPARATION_STRING);
		strcat(certdbpath, cadir);
	}
	else {
		certdbpath = (char *)malloc(strlen(cadir)+strlen(CERTDB)+2);
		if (!certdbpath) {
			aux_add_error(EMALLOC, "certdbpath", CNULL, 0, proc);
			return ((Certificate *)0);
		}
		strcpy(certdbpath, cadir);
	}
	if (certdbpath[strlen(certdbpath) - 1] != PATH_SEPARATION_CHAR) strcat(certdbpath, PATH_SEPARATION_STRING);
	strcat(certdbpath, CERTDB);

#ifdef NDBM
	cert = dbm_open(certdbpath, O_RDONLY, 0);
	if (!cert) {
		aux_add_error(EINVALID, "Can't open certificate database", CNULL, 0, proc);
		free(certdbpath);
		return ((Certificate *)0);
	}
#else
	 {
		FILE * fd;
		char	fn[64];
		strcpy(fn, certdbpath);
		strcat(fn, ".pag");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		strcpy(fn, certdbpath);
		strcat(fn, ".dir");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		if (dbminit(certdbpath) < 0) {
			aux_add_error(EINVALID, "Can't open certificate database", CNULL, 0, proc);
			free(certdbpath);
			return ((Certificate *)0);
		}

	}
#endif
	free(certdbpath);
	key.dptr = serial->octets;
	key.dsize = serial->noctets;

#ifdef NDBM
	data = dbm_fetch(cert, key);
#else
	data = fetch(key);
#endif
	if (!data.dptr)	 {
		aux_add_error(EINVALID, "Can't get data from certificate database", CNULL, 0, proc);
		return ((Certificate *)0);
	}

	ostr = (OctetString *) malloc(sizeof(OctetString));
	if( !ostr ) {
		aux_add_error(EMALLOC, "ostr", CNULL, 0, proc);
		return ((Certificate *)0);
	}
	ostr->noctets = data.dsize;
	ostr->octets = data.dptr;
	certificate = d_Certificate(ostr);
	free(ostr);
#ifdef NDBM
	dbm_close(cert);
#else
	dbmclose();
#endif

	return (certificate);
}


/***************************************************************
 *
 * Procedure af_cadb_list_user
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_Name *af_cadb_list_user(
	char	 *cadir
)

#else

SET_OF_Name *af_cadb_list_user(
	cadir
)
char	 *cadir;

#endif

{
#ifdef NDBM
	DBM	    * user;
#endif
	datum	      key;
	SET_OF_Name * nameset, * tmp_nameset;
	char 	    * userdbpath;
	char        * homedir;

	char        * proc = "af_cadb_list_user";


	if (!cadir) {
		aux_add_error(EINVALID, "Can't get users list from database because no CA directory specified", CNULL, 0, proc);
		return ((SET_OF_Name *)0);
	}

	if(cadir[0] != PATH_SEPARATION_CHAR) {
		homedir = getenv("HOME");
		userdbpath = (char *)malloc(strlen(homedir)+strlen(cadir)+strlen(USERDB)+3);
		if (!userdbpath) {
			aux_add_error(EMALLOC, "userdbpath", CNULL, 0, proc);
			return ((SET_OF_Name *)0);
		}
		strcpy(userdbpath, homedir);
		strcat(userdbpath, PATH_SEPARATION_STRING);
		strcat(userdbpath, cadir);
	}
	else {
		userdbpath = (char *)malloc(strlen(cadir)+strlen(USERDB)+2);
		if (!userdbpath) {
			aux_add_error(EMALLOC, "userdbpath", CNULL, 0, proc);
			return ((SET_OF_Name *)0);
		}
		strcpy(userdbpath, cadir);
	}
	if (userdbpath[strlen(userdbpath) - 1] != PATH_SEPARATION_CHAR) strcat(userdbpath, PATH_SEPARATION_STRING);
	strcat(userdbpath, USERDB);

#ifdef NDBM
	user = dbm_open(userdbpath, O_RDONLY, 0);
	if (!user) {
		aux_add_error(EINVALID, "Can't open user database", CNULL, 0, proc);
		free(userdbpath);
		return ((SET_OF_Name *)0);
	}
#else
	 {
		FILE * fd;
		char	fn[64];
		strcpy(fn, userdbpath);
		strcat(fn, ".pag");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		strcpy(fn, userdbpath);
		strcat(fn, ".dir");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		if (dbminit(userdbpath) < 0) {
			aux_add_error(EINVALID, "Can't open user database", CNULL, 0, proc);
			free(userdbpath);
			return ((SET_OF_Name *)0);
		}

	}
#endif
	free(userdbpath);
	nameset = tmp_nameset = (SET_OF_Name *) malloc(sizeof(SET_OF_Name));
	if( !tmp_nameset ) {
		aux_add_error(EMALLOC, "tmp_nameset", CNULL, 0, proc);
		return ((SET_OF_Name *)0);
	}

#ifdef NDBM
	key = dbm_firstkey(user);
#else
	key = dbm_firstkey();
#endif

	if (!key.dptr) {
		free(nameset);
		return ((SET_OF_Name *)0);
	}

	tmp_nameset->element = malloc(key.dsize + 1);
	if( !tmp_nameset->element ) {
		aux_add_error(EMALLOC, "tmp_nameset->element", CNULL, 0, proc);
		return ((SET_OF_Name *)0);
	}
	bcopy(key.dptr, conv_to_upper(tmp_nameset->element), key.dsize);
	*(tmp_nameset->element + key.dsize) = '\0';
	tmp_nameset->next = (SET_OF_Name *)0;

#ifdef NDBM
	key = dbm_nextkey(user);
#else
	key = dbm_nextkey();
#endif

	while (key.dptr != CNULL) {
		tmp_nameset->next = (SET_OF_Name *) malloc(sizeof(SET_OF_Name));
		if( !tmp_nameset->next ) {
			aux_add_error(EMALLOC, "tmp_nameset->next", CNULL, 0, proc);
			return ((SET_OF_Name *)0);
		}
		tmp_nameset = tmp_nameset->next;
		tmp_nameset->element = malloc(key.dsize + 1);
		if( !tmp_nameset->element ) {
			aux_add_error(EMALLOC, "tmp_nameset->element", CNULL, 0, proc);
			return ((SET_OF_Name *)0);
		}
		bcopy(key.dptr, conv_to_upper(tmp_nameset->element), key.dsize);
		*(tmp_nameset->element + key.dsize) = '\0';
		tmp_nameset->next = (SET_OF_Name *)0;

#ifdef NDBM
		key = dbm_nextkey(user);
#else
		key = dbm_nextkey();
#endif
	}

#ifdef NDBM
	dbm_close(user);
#else
	dbmclose();
#endif

	return (nameset);
}


/***************************************************************
 *
 * Procedure logtime
 *
 ***************************************************************/
#ifdef __STDC__

char *logtime(
)

#else

char *logtime(
)

#endif

{
	static char	tbuf[20];
	struct tm *now;
	time_t	intnow;

	intnow = time(0);
	now = localtime(&intnow);
	sprintf(tbuf, "%02d/%02d/%02d %02d:%02d:%02d",
	    now->tm_mon + 1,
	    now->tm_mday,
	    now->tm_year,
	    now->tm_hour,
	    now->tm_min,
	    now->tm_sec);

	return tbuf;
}

/***************************************************************
 *
 * Procedure conv_to_upper
 *
 ***************************************************************/
#ifdef __STDC__

static char *conv_to_upper(
	char	 *c
)

#else

static char *conv_to_upper(
	c
)
char	 *c;

#endif

{
        register char *cc = c;
        while(*cc) {
                if(*cc >= 'a' && *cc <= 'z') *cc -= 32;
                cc++;
        }
        return(c);
}


/***************************************************************
 *
 * Procedure af_cadb_add_CRLWithCertificates
 *
 ***************************************************************/
#ifdef __STDC__

int af_cadb_add_CRLWithCertificates(
	CRLWithCertificates	 *crlwithcerts,
	char		 *cadir
)

#else

int af_cadb_add_CRLWithCertificates(
	crlwithcerts,
	cadir
)
CRLWithCertificates	 *crlwithcerts;
char		 *cadir;

#endif

{
#ifdef NDBM
	DBM	    * crl;
#endif
	datum	      key, data;
	OctetString * oct_crlwithcerts;
	int	      i, * p, rc;
	char        * dbpath, * crldbpath, * homedir;
	Name	    * cname;

	char        * proc = "af_cadb_add_CRLWithCertificates";


	if (! cadir) {
		aux_add_error(EINVALID, "Can't add revocation list to database because no CA directory specified", CNULL, 0, proc);
		return (-1);
	}

	if(cadir[0] != PATH_SEPARATION_CHAR) {
		homedir = getenv("HOME");
		dbpath = (char *)malloc(strlen(homedir)+strlen(cadir)+3);
		if (!dbpath) {
			aux_add_error(EMALLOC, "dbpath", CNULL, 0, proc);
			return (- 1);
		}
		strcpy(dbpath, homedir);
		strcat(dbpath, PATH_SEPARATION_STRING);
		strcat(dbpath, cadir);
	}
	else {
		dbpath = (char *)malloc(strlen(cadir)+2);
		if (!dbpath) {
			aux_add_error(EMALLOC, "dbpath", CNULL, 0, proc);
			return (- 1);
		}
		strcpy(dbpath, cadir);
	}
	if (dbpath[strlen(dbpath) - 1] != PATH_SEPARATION_CHAR) strcat(dbpath, PATH_SEPARATION_STRING);

	crldbpath = (char *)malloc(strlen(dbpath)+strlen(CRLDB)+1);
	strcpy(crldbpath, dbpath);
	strcat(crldbpath, CRLDB);
	free(dbpath);

	/*	store new crl entry */
#ifdef NDBM
	crl = dbm_open(crldbpath, O_RDWR, 0);
	if (! crl){
		aux_add_error(EINVALID, "Can't open revocation list database", CNULL, 0, proc);
		free(crldbpath);
		return(1);
	}
#else
	 {
		FILE  * fd;
		char	fn[64];

		strcpy(fn, crldbpath);
		strcat(fn, ".pag");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		strcpy(fn, crldbpath);
		strcat(fn, ".dir");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		if (dbminit(crldbpath) < 0) {
			aux_add_error(EINVALID, "Can't open revocation list database", CNULL, 0, proc);
			free(crldbpath);
			return(1);
		}

	}
#endif
	free(crldbpath);

	oct_crlwithcerts = e_CRLWithCertificates(crlwithcerts);
	if (! oct_crlwithcerts)  {
		aux_add_error(EENCODE, "e_CRLWithCertificates failed", CNULL, 0, proc);
		return(-1);
	}

	cname = aux_DName2CAPITALName(crlwithcerts->crl->tbs->issuer);
	if(! cname){
		aux_add_error(EINVALID, "aux_DName2Name failed", CNULL, 0, proc);
		return(-1);
	}

	key.dptr = cname;
	key.dsize = strlen(key.dptr);
	data.dptr = oct_crlwithcerts->octets;
	data.dsize = oct_crlwithcerts->noctets;

#ifdef NDBM
	rc = dbm_store(crl, key, data, DBM_INSERT);
	if (rc < 0) {
		aux_free_OctetString(&oct_crlwithcerts);
		free(cname);
		aux_add_error(EINVALID, "Can't store data to revocation list database", CNULL, 0, proc);
		return(rc);
	}
	if (rc == 1){
		/* see manual:							*/
		/* All functions that return an int indicate errors with  nega- */
		/* tive  values.   A  zero return indicates no error.  Routines */
		/* that return a datum indicate errors with a  NULL  (0)  dptr. */
		/* If  dbm_store  called with a flags value of DBM_INSERT finds */
		/* an existing entry with the same key it returns 1.     	*/

		rc = dbm_store(crl, key, data, DBM_REPLACE);
		if (rc < 0) {
			aux_free_OctetString(&oct_crlwithcerts);
			free(cname);
			aux_add_error(EINVALID, "Can't store data to revocation list database", CNULL, 0, proc);
			return(rc);
		}
	}
#else
	rc = store(key, data);
	if(rc < 0){
		aux_free_OctetString(&oct_crlwithcerts);
		free(cname);
		aux_add_error(EINVALID, "Can't store data to revocation list database", CNULL, 0, proc);
		return(rc);
	}
#endif
	free(cname);
	LOGPEMCRL(crlwithcerts);

#ifdef NDBM
	dbm_close(crl);
#else
	dbmclose();
#endif
	aux_free_OctetString(&oct_crlwithcerts);

	return(0);
}



/***************************************************************
 *
 * Procedure af_cadb_list_CRLWithCertificates
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_CRLWithCertificates *af_cadb_list_CRLWithCertificates(
	char	 *cadir
)

#else

SET_OF_CRLWithCertificates *af_cadb_list_CRLWithCertificates(
	cadir
)
char	 *cadir;

#endif

{
#ifdef NDBM
	DBM	               * crl;
#endif
	SET_OF_CRLWithCertificates * set, * tmp_set;
	char 	               * crldbpath, * homedir;
	datum	                 key, data;
	OctetString	       * ostr;

	char                   * proc = "af_cadb_list_CRLWithCertificates";


	if (! cadir) {
		aux_add_error(EINVALID, "Can't get revocation lists from database because no CA directory specified", CNULL, 0, proc);
		return ((SET_OF_CRLWithCertificates *)0);
	}

	if(cadir[0] != PATH_SEPARATION_CHAR) {
		homedir = getenv("HOME");
		crldbpath = (char *)malloc(strlen(homedir)+strlen(cadir)+strlen(CRLDB)+3);
		if (!crldbpath) {
			aux_add_error(EMALLOC, "crldbpath", CNULL, 0, proc);
			return ((SET_OF_CRLWithCertificates *)0);
		}
		strcpy(crldbpath, homedir);
		strcat(crldbpath, PATH_SEPARATION_STRING);
		strcat(crldbpath, cadir);
	}
	else {
		crldbpath = (char *)malloc(strlen(cadir)+strlen(CRLDB)+2);
		if (!crldbpath) {
			aux_add_error(EMALLOC, "crldbpath", CNULL, 0, proc);
			return ((SET_OF_CRLWithCertificates *)0);
		}
		strcpy(crldbpath, cadir);
	}
	if (crldbpath[strlen(crldbpath) - 1] != PATH_SEPARATION_CHAR) strcat(crldbpath, PATH_SEPARATION_STRING);
	strcat(crldbpath, CRLDB);

#ifdef NDBM
	crl = dbm_open(crldbpath, O_RDONLY, 0);
	if (!crl) {
		aux_add_error(EINVALID, "Can't open revocation list database", CNULL, 0, proc);
		free(crldbpath);
		return ((SET_OF_CRLWithCertificates *)0);
	}
#else
	 {
		FILE * fd;
		char	fn[64];
		strcpy(fn, crldbpath);
		strcat(fn, ".pag");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		strcpy(fn, crldbpath);
		strcat(fn, ".dir");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		if (dbminit(crldbpath) < 0) {
			aux_add_error(EINVALID, "Can't open revocation list database", CNULL, 0, proc);
			free(crldbpath);
			return ((SET_OF_CRLWithCertificates *)0);
		}

	}
#endif
	free(crldbpath);
	set = tmp_set = (SET_OF_CRLWithCertificates * )malloc(sizeof(SET_OF_CRLWithCertificates));
	if(! tmp_set) {
		aux_add_error(EMALLOC, "tmp_set", CNULL, 0, proc);
		return ((SET_OF_CRLWithCertificates *)0);
	}

#ifdef NDBM
	key = dbm_firstkey(crl);
#else
	key = dbm_firstkey();
#endif

	if (! key.dptr){
		aux_free_SET_OF_CRLWithCertificates(&set);
		return ((SET_OF_CRLWithCertificates * )0);
	}

#ifdef NDBM
	data = dbm_fetch(crl, key);
#else
	data = fetch(key);
#endif
	if (! data.dptr){
		aux_add_error(EINVALID, "Can't get data from revocation list database", CNULL, 0, proc);
		return ((SET_OF_CRLWithCertificates *)0);
	}
	ostr = (OctetString * )malloc(sizeof(OctetString));
	if(! ostr){
		aux_add_error(EMALLOC, "ostr", CNULL, 0, proc);
		return ((SET_OF_CRLWithCertificates *)0);
	}
	ostr->noctets = data.dsize;
	ostr->octets = data.dptr;
	tmp_set->element = d_CRLWithCertificates(ostr);
	free(ostr);
	if(! tmp_set->element){
		aux_add_error(EDECODE, "d_CRLWithCertificates failed", CNULL, 0, proc);
		aux_free_SET_OF_CRLWithCertificates(&set);
		return ((SET_OF_CRLWithCertificates *)0);
	}
	tmp_set->next = (SET_OF_CRLWithCertificates *)0;

#ifdef NDBM
	key = dbm_nextkey(crl);
#else
	key = dbm_nextkey();
#endif

	while (key.dptr != CNULL) {
#ifdef NDBM
		data = dbm_fetch(crl, key);
#else
		data = fetch(key);
#endif
		tmp_set->next = (SET_OF_CRLWithCertificates *) malloc(sizeof(SET_OF_CRLWithCertificates));
		if(! tmp_set->next){
			aux_add_error(EMALLOC, "tmp_set->next", CNULL, 0, proc);
			return ((SET_OF_CRLWithCertificates * )0);
		}
		tmp_set = tmp_set->next;
		ostr = (OctetString * )malloc(sizeof(OctetString));
		if(! ostr){
			aux_add_error(EMALLOC, "ostr", CNULL, 0, proc);
			return ((SET_OF_CRLWithCertificates *)0);
		}
		ostr->noctets = data.dsize;
		ostr->octets = data.dptr;
		tmp_set->element = d_CRLWithCertificates(ostr);
		free(ostr);
		if(! tmp_set->element){
			aux_add_error(EDECODE, "d_CRLWithCertificates failed", CNULL, 0, proc);
			aux_free_SET_OF_CRLWithCertificates(&set);
			return ((SET_OF_CRLWithCertificates *)0);
		}
		tmp_set->next = (SET_OF_CRLWithCertificates *)0;

#ifdef NDBM
		key = dbm_nextkey(crl);
#else
		key = dbm_nextkey();
#endif
	}

#ifdef NDBM
	dbm_close(crl);
#else
	dbmclose();
#endif

	return (set);
}


/***************************************************************
 *
 * Procedure af_cadb_get_CRLWithCertificates
 *
 ***************************************************************/
#ifdef __STDC__

CRLWithCertificates *af_cadb_get_CRLWithCertificates(
	Name	 *name,
	char	 *cadir
)

#else

CRLWithCertificates *af_cadb_get_CRLWithCertificates(
	name,
	cadir
)
Name	 *name;
char	 *cadir;

#endif

{
#ifdef NDBM
	DBM	             * crl;
#endif
	CRLWithCertificates      * crlwithcerts;
	OctetString          * ostr;
	char	             * crldbpath;
	char   		     * homedir;
	DName		     * dname;
	Name                 * cname;
	datum		       key, data;

	char		     * proc = "af_cadb_get_CRLWithCertificates";


	if (! cadir) {
		aux_add_error(EINVALID, "Can't get revocation list from database because no CA directory specified", CNULL, 0, proc);
		return ((CRLWithCertificates *)0);
	}

	if(cadir[0] != PATH_SEPARATION_CHAR) {
		homedir = getenv("HOME");
		crldbpath = (char *)malloc(strlen(homedir)+strlen(cadir)+strlen(CRLDB)+3);
		if (!crldbpath) {
			aux_add_error(EMALLOC, "crldbpath", CNULL, 0, proc);
			return ((CRLWithCertificates *)0);
		}
		strcpy(crldbpath, homedir);
		strcat(crldbpath, PATH_SEPARATION_STRING);
		strcat(crldbpath, cadir);
	}
	else {
		crldbpath = (char *)malloc(strlen(cadir)+strlen(CRLDB)+2);
		if (!crldbpath) {
			aux_add_error(EMALLOC, "crldbpath", CNULL, 0, proc);
			return ((CRLWithCertificates *)0);
		}
		strcpy(crldbpath, cadir);
	}
	if (crldbpath[strlen(crldbpath) - 1] != PATH_SEPARATION_CHAR) strcat(crldbpath, PATH_SEPARATION_STRING);
	strcat(crldbpath, CRLDB);


	/*	fetch subject crl record */
 
 	if (!name) dname = af_pse_get_Name();		
	else dname = aux_Name2DName(name);
	
	if(! dname) {
		aux_add_error(EINVALID, "Can't get revocation list from database because name is wrong", name, char_n, proc);
		return((CRLWithCertificates *)0);
	}		
	cname = aux_DName2CAPITALName(dname);
	aux_free_DName(&dname);

#ifdef NDBM
	crl = dbm_open(crldbpath, O_RDONLY, 0);
	free(crldbpath);
	if (! crl) {
		aux_add_error(EINVALID, "Can't open revocation list database", CNULL, 0, proc);
		return ((CRLWithCertificates *)0);
	}
#else
	 {
		FILE * fd;
		char	fn[64];
		strcpy(fn, crldbpath);
		strcat(fn, ".pag");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		strcpy(fn, crldbpath);
		strcat(fn, ".dir");
		fd = fopen(fn, "r");
		if (fd) fclose(fd);
		else {
			fd = fopen(fn, "w");
			fclose(fd);
		}
		if (dbminit(crldbpath) < 0) {
			aux_add_error(EINVALID, "Can't open revocation list database", CNULL, 0, proc);
			return ((CRLWithCertificates *)0);
		}
	}
#endif
	key.dptr = cname;
	key.dsize = strlen(key.dptr);
#ifdef NDBM
	data = dbm_fetch(crl, key);
#else
	data = fetch(key);
#endif
	free(cname);

	if (! data.dptr){
		aux_add_error(EINVALID, "Can't get data from revocation list database", CNULL, 0, proc);
		return ((CRLWithCertificates *)0);
	}
	ostr = (OctetString *) malloc(sizeof(OctetString));
	if(! ostr) {
		aux_add_error(EMALLOC, "ostr", CNULL, 0, proc);
		return ((CRLWithCertificates *)0);
	}
	ostr->noctets = data.dsize;
	ostr->octets = data.dptr;
	crlwithcerts = d_CRLWithCertificates(ostr);
	free(ostr);
	if(! crlwithcerts){
		aux_add_error(EDECODE, "d_CRLWithCertificates failed", CNULL, 0, proc);
		return ((CRLWithCertificates *)0);
	}
#ifdef NDBM
	dbm_close(crl);
#else
	dbmclose();
#endif

	return (crlwithcerts);
}
