/*********************************************************************
 * Filename:      keybase.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Mon Jan 19 10:19:41 1998
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/


/*
 * In order to keep track of encryption keys we store them in a set
 * of databases.  The "real" index is the KeyID (64 bit long) and
 * you can use this to find a key based on the KeyID.
 *
 * Then, the string(s) associated with each key ("User ID Packet" in
 * PGP terms) are stored in a separate database for faster lookup.
 */

/* A name must be provided.  From it, we will create two file-names.
 * If these files exists, we will use them, if not, we will create them.
 */

#include <pgplib.h>


#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_DB_H
#  include <db.h>
#endif

#include <md5.h>

#if (HAVE_STRERROR == 0)
extern char *  strerror (int);
#endif

#define KEYEXT		".key"
#define STRINGEXT	".string"
#define	REVSTRINGEXT	".revstring"
#define KEYIDEXT	".keyid"
#define KEYHASHEXT	".hash"

static DB	*keydb;
static DB	*stringdb;
static DB	*revstringdb;
static DB	*keyiddb;
static DB	*keyhashdb;

int	dbase_open = 0;


void
close_databases()
{
    pgplib_errno = PGPLIB_ERROR_DB_FATAL;
    exit(1);
}

int
keydb_init(char 	*caller_name,
	   openmode_t	caller_mode)
{
    char 	filename_b[MAXPATHLEN];
    char	*filename;
    char 	*p;
    int		mode;
    HASHINFO	*hinfo;

    if(dbase_open)
      return 0;

    if (caller_name == NULL)
	filename = NULL;
    else
	filename = filename_b;

    switch(caller_mode) {
    case READER:
	mode = O_RDONLY;
	break;
    case WRITER:
	mode = O_RDWR;
	break;
    case NEWDB:
	mode = O_CREAT | O_TRUNC | O_RDWR ;
	break;
    default:
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    keydb = NULL; stringdb = NULL; revstringdb = NULL; keyiddb = NULL;

#ifdef NotDef

    You need to tune these to meet your requirement, in particular,
    how much memory you have available for the system.
    Be careful, you can easily get a 100 MB process if you have a lot of
    keys at hand.

    hinfo = calloc(1, sizeof(*hinfo));
    if ( hinfo == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    hinfo->cachesize = 5*CACHE_SIZE;
    hinfo->bsize     = BSIZE;
    hinfo->nelem     = NELEM;
#else
    hinfo = NULL;
#endif


    if(filename) {
	filename[0] = '\0';
	strcat(filename, caller_name);
	strcat(filename, KEYEXT);
    }

    keydb = dbopen (filename, mode, 0600, DB_HASH, hinfo);
    if ( keydb == NULL )
	goto err;

#ifdef NotDef
    More memory tuning

    hinfo.cachesize = CACHE_SIZE;
    hinfo.bsize     = BSIZE;
#endif

    if(filename) {
	p = strrchr(filename, '.');
	*p = '\0';
	strcat(filename, STRINGEXT);
    }
    stringdb = dbopen(filename, mode, 0600, DB_HASH, hinfo);
    if ( stringdb == NULL )
	goto err;

    if(filename) {
	p = strrchr(filename, '.');
	*p = '\0';
	strcat(filename, REVSTRINGEXT);
    }
    revstringdb = dbopen (filename, mode, 0600, DB_HASH, hinfo);
    if (revstringdb == NULL )
	goto err;

    if(filename) {
	p = strrchr(filename, '.');
	*p = '\0';
	strcat(filename, KEYIDEXT);
    }
    keyiddb = dbopen (filename, mode, 0600, DB_HASH, hinfo);
    if ( keyiddb == NULL )
	goto err;

    if(filename) {
	p = strrchr(filename, '.');
	strcpy(p, KEYHASHEXT);
    }
    keyhashdb = dbopen(filename, mode, 0600, DB_HASH, hinfo);

    dbase_open = 1;
    return(0);

err:
    keydb_end();
    pgplib_errno = PGPLIB_ERROR_DB_FATAL;
    return -1;
}


void
keydb_end(void)
{
    if ( keydb != NULL )
	(keydb->close)(keydb);
    if ( stringdb !=  NULL )
	(stringdb->close)(stringdb);
    if ( revstringdb != NULL )
	(revstringdb->close)(revstringdb);
    if ( keyiddb != NULL )
	(keyiddb->close)(keyiddb);
    if(keyhashdb != NULL)
	keyhashdb->close(keyhashdb);
    dbase_open = 0;
    keydb = stringdb = revstringdb = keyiddb = NULL;
}

void
keydb_sync(void)
{
    if ( keydb != NULL )
	(keydb->sync)(keydb, 0);
    if ( stringdb !=  NULL )
	(stringdb->sync)(stringdb, 0);
    if ( revstringdb != NULL )
	(revstringdb->sync)(revstringdb, 0);
    if ( keyiddb != NULL )
	(keyiddb->sync)(keyiddb, 0);
    if(keyhashdb != NULL)
	keyhashdb->sync(keyhashdb, 0);
}

/* Below is all the routines to store, fetch, alter and delete
 * PGP compatible keys and certificates.
 * All these routines take and return nicely formatted keys and certificates
 * as pointers to struct's.  The internal format (in the databases)
 * are less nice since it has to be marshalled (and, urk, compatible).
 *
 * In the Key database, the keys (together with UserID and signatures)
 * are stored in PGP format.  This way we can print the
 * key fast by simply "armour" the buffer directly.  It takes some
 * effort to convert the buffer to the correct type of C-struct's,
 * but that is the price we have to pay.
 *
 * The strings (UserID) are stored in a separate database.  This
 * way we can locate one _very_ fast
 *
 * The KeyID that pgp presents (only 32 bit) are stored in a third
 * database.
 */

/*
 * Take a string ("0x12345678") and convert it into the corresponding
 * number, and find the key.  No key: return 1 and set keyid to 0.
 * On error return -1;
 */

int
find_shortid(char 	*short_string,
	     PGPKEYID	*keyid)
{

    DBT		key, contents;
    u_long	short_id;
    int 	i;
    char	*p;

    if ( ! dbase_open ) {
	pgplib_errno = PGPLIB_ERROR_DB_NOT_OPEN;
	return(-1);
    }

    if ( short_string == NULL || *short_string == 0 || keyid == NULL) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    PGPKEYID_ZERO(*keyid);

    short_id = strtoul(short_string, &p, 0);
    if ( p == NULL || *p != 0 ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    key.data = (void *)&short_id;
    key.size = sizeof(short_id);

    i = (keyiddb->get)(keyiddb, &key, &contents, 0);
    if ( i == -1 ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    if ( i == 0 ) {
	/* fixed as it was  == which logically is wrong if you look
	 * at the code --tjh 
	 */
	if (contents.size != sizeof(PGPKEYID)) {
	    pgplib_errno = pgplib_errno = PGPLIB_ERROR_IMPOSSIBLE;
	    return(-1);
	}
	memcpy(keyid, contents.data, sizeof(PGPKEYID));
    }
    return(0);
}

int
store_pubkey(PGPpubkey_t	*pubkey,
	      repl_t		replace)
{
    DBT		key, contents;
    PGPKEYID	key_id;
    u_long	short_id;
    int		key_size;
    u_char	*key_buf;
    /* scratch */
    int 	i;

    if ( ! dbopen ) {
	pgplib_errno = PGPLIB_ERROR_DB_NOT_OPEN;
	return(-1);
    }

    if ((replace != INSERT && replace != REPLACE) || pubkey == NULL) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }
    if ( ! PGPKEYID_FromBN(&key_id,pubkey->N)) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    i = pubkey2buf(pubkey,  &key_buf);
    if ( i != 0 ) {
	return(-1);
    }

    key_size = packet_length(key_buf);
    if ( key_size == -1 )
	return(-1);

    i = store_pubkey_rec(key_id, replace, key_buf, key_size);
    free(key_buf);
    switch (i) {
    case -1:
	/*FALLTHROUGH*/
    case 1:
	return(i);
    case 0:
	break;
    default:
	pgplib_errno = PGPLIB_ERROR_IMPOSSIBLE;
	return (-1);
    }

    /* The store went well.  We then proceed by adding the shorter
     * id presented by "pgp -kv", a string representing a 32 bit
     * number.  However, we store the string as the number it is
     * supposed to represent (occupies less than half the space)
     */   

    PGPKEYID_ExtractShort(&key_id, &short_id);

    key.data = (void *)&short_id;
    key.size = sizeof(short_id);

    contents.data = (void *) &key_id;
    contents.size= sizeof(key_id);

    i = (keyiddb->put)(keyiddb, &key, &contents, 0);
    if ( i == -1 ) {
	pgplib_errno = PGPLIB_ERROR_DB_FATAL;
	return(-1);
    }
    else
	if (i == 1)
	    /* R_NOOVERWRITE flag was set and the key already exists.
	     * This case is identified for further modification, if
	     * required.
	     */

           ;

    return(0);
}

int
get_pubkey_rec(PGPKEYID	key_id,
	       int	*len,
	       u_char 	**buf)
{

    DBT		key, contents;
    int		i;


    if ( ! dbase_open ) {
	pgplib_errno = PGPLIB_ERROR_DB_NOT_OPEN;
	return(-1);
    }

    if ( PGPKEYID_IsZero(key_id) || len == NULL || buf == NULL) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    key.data = (void *)&key_id;
    key.size = sizeof(key_id);

    i = (keydb->get)(keydb, &key, &contents, 0);
    if ( i != 0 ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return( i );
    }

    *buf = malloc(contents.size);
    if ( *buf == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }

    memcpy(*buf, contents.data, contents.size);
    *len = contents.size;

    return(0);
}

int
store_pubkey_rec(PGPKEYID	key_id,
		 repl_t 	user_replace,
		 u_char		*buf,
		 int 		size)
{
    DBT 	key, contents;
    int 	i,j;
    u_char	digest[16];

    if ( ! dbase_open) {
	pgplib_errno = PGPLIB_ERROR_DB_NOT_OPEN;
	return(-1);
    }

    if ((user_replace != REPLACE  && user_replace != INSERT) || buf == NULL
	|| size < 1)
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    key.data = (void *)&key_id;
    key.size = sizeof(key_id);

    contents.data = (void *)buf;
    contents.size = size;

    if ( user_replace == REPLACE )
	j = 0;
    else
	j = R_NOOVERWRITE;

    assert(contents.size > 0);
    assert(key.size > 0);
    assert(contents.data != NULL);
    assert(key.data != NULL);

    i = (keydb->put)(keydb, &key, &contents, j);
    if(i == 0)
    {
	MD5(buf, size, digest);
	key.data = &key_id;
	key.size = sizeof(key_id);
	contents.data = digest;
	contents.size = sizeof(digest);
	keyhashdb->put(keyhashdb, &key, &contents, j);
    }

    return( i );
}

/*
 * If the key exists, return 1.  If not, return 0.  Error: -1
 */

int
key_exists(PGPKEYID	key_id)
{
    DBT		key, contents;
    int 	i;

    if ( ! dbase_open ) {
	pgplib_errno = PGPLIB_ERROR_DB_NOT_OPEN;
	return(-1);
    }

    key.data = (void *)&key_id;
    key.size = sizeof(key_id);

    i = (keydb->get)(keydb, &key, &contents, 0);
    if ( i == -1 ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    return ( 1 - i );
}

/*
 * Storing a UID consists of two actions.  First, store the KeyID
 * indexed by the UID.  Then, store the UID indexed by the KeyID.
 */

int
store_uid(PGPuserid_t	*user_id,
	  PGPKEYID 	key_id)
{
    DBT		key, contents;
    res_t	what;
    int		used, how_far, found, newlen, ret;
    PGPuserid_t	*uid;
    u_char	*buf;
    /* scratch */
    int 	i;
    u_char	*p;
    void	*tmpptr;

    if ( ! dbase_open ) {
	pgplib_errno = PGPLIB_ERROR_DB_NOT_OPEN;
	return(-1);
    }

    if ( user_id == NULL || PGPKEYID_IsZero(key_id)) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    key.data = (void *)user_id->name;
    key.size = strlen(user_id->name) + 1;

    found = 0;

    if(!stringdb->get(stringdb, &key, &contents, 0))
    {
	void *buf;
	int j;

	for (j = 0; !found && j * sizeof(PGPKEYID) < contents.size; j++)
	    if (PGPKEYID_EQUAL(((PGPKEYID *)contents.data)[j],key_id))
		found = 1;

	if (!found)
	{
	    if ( ! (buf = malloc(contents.size + sizeof(key_id))))
	    {
		pgplib_errno = PGPLIB_ERROR_SYSERR;
		return -1;
	    }

	    memcpy(buf, contents.data, contents.size);
	    memcpy((char *)buf + contents.size, &key_id, sizeof(key_id));
	    contents.size += sizeof(key_id);
	    tmpptr = contents.data = buf;
	}
    }

    if ( ! found)
    {
	if ( ! (tmpptr = contents.data = malloc(sizeof(key_id))))
	{
	    pgplib_errno = PGPLIB_ERROR_SYSERR;
	    return -1;
	}
	memcpy(contents.data, (void *) &key_id, sizeof(key_id));
	contents.size = sizeof(key_id);

	key.data = (void *) user_id->name;
	key.size = strlen(user_id->name)+1;
	
	if((contents.size < 1 ) || (key.size < 1))
	{
	    pgplib_errno = PGPLIB_ERROR_EINVAL;
	    return -1;
	}
	
	assert(contents.data != NULL);
	assert(key.data != NULL);

	i = (stringdb->put)(stringdb, &key, &contents, 0);
	if ( i == -1 ) {
	    pgplib_errno = PGPLIB_ERROR_SYSERR;
	    return(-1);
	}

	free(tmpptr);
	
    }

    key.data= (void *) &key_id;
    key.size = sizeof(key_id);

    ret = (revstringdb->get)(revstringdb, &key, &contents, 0);
    switch (ret) {
    default:
	assert(0);
    case -1:
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    case 1:
	/* This is the first UID for this key */
	i = uid2buf(user_id, (u_char **) &tmpptr);
	contents.data = tmpptr;
	if ( i != 0 )
	    return(-1);
	contents.size = packet_length((u_char*)tmpptr);
	if ( contents.size == -1 )
	    return(-1);
	i = (revstringdb->put)(revstringdb, &key, &contents, 0);
	free(tmpptr);
	if ( i != 0 ) {
	    pgplib_errno = PGPLIB_ERROR_SYSERR;
	    return(-1);
	}
	else
	    return(0);
	/*NOTREACHED*/
	assert(0);
    case 0:
	/* There was already (at least) one UID associated with
	 * this key.  We have to walk through them to check if
	 * the one we have already is here.
	 * Note that we (should) have a PGP compatile buffer at hand.
	 */
	how_far = 0;
	found = 0;
	while( how_far < contents.size  && found == 0) {

	    i = get_keyentry_from_buf( (u_char *)contents.data  + how_far,
				       contents.size - how_far,
				       &what, NULL, NULL, &uid, NULL, NULL,
				       &used);
	    if ( i != 0 ) {
		return(-1);
	    }

	    /*
	     * The string DB should not contain anything but UID's.
	     */
	    assert( what == UID);

	    how_far += used;

	    if ( strcmp(user_id->name, uid->name) == 0 ) {
		found = 1;
	    }
	    free_uid(uid);
	}
	if ( found == 1 )
	    return(1);

	if ( uid2buf(user_id, &buf) != 0 )
	    return(-1);

	if ( (newlen = packet_length(buf)) == -1 )
	    return -1;

	if ( (p = malloc(contents.size + newlen)) == NULL ) {
	    pgplib_errno = PGPLIB_ERROR_SYSERR;
	    return(-1);
	}
	memcpy(p, contents.data, contents.size);
	memcpy(p+contents.size, buf, newlen);
	free(buf);
	contents.data = p;
	contents.size += newlen;

	i = (revstringdb->put)(revstringdb, &key, &contents, 0);
	free(p);
	if ( i != 0 ) {
	    pgplib_errno = PGPLIB_ERROR_SYSERR;
	    return(-1);
	}
	return(0);
    }
    assert(0);
    return(0);
    /*NOTREACHED*/
}


int
keys_from_function(int (*func)(const void *, const char *uid),
		   void *info,
		   int *num_keys,
		   PGPKEYID **keys)
{
    DBT key, contents;
    char *n = NULL;
    char *t;
    int nl = 0;
    int rv;
    int alloced = 0;
    PGPKEYID *tmp;

    *num_keys = 0;
    *keys = NULL;

    rv = stringdb->seq(stringdb, &key, &contents, R_FIRST);
    while(rv == 0)
    {
	if(key.size + 1 >= nl)
	{
	    if((t = realloc(n, (nl = key.size + 1))) == NULL)
	    {
		free(n);
		pgplib_errno = PGPLIB_ERROR_SYSERR;
		*keys = NULL;
		*num_keys = 0;
		return -1;
	    }
	    n = t;
	}

	memcpy(n, key.data, key.size);
	n[key.size] = '\0';
	
	if(func(info, n))
	{
	    int j;
	    for(j = 0; j * sizeof(PGPKEYID) < contents.size; j++)
	    {
		if(*num_keys >= alloced)
		{
		    if((tmp = realloc(*keys, (alloced += 128) * sizeof(PGPKEYID))) == NULL)
		    {
			pgplib_errno = PGPLIB_ERROR_SYSERR;
			if(*keys) free(*keys);
			*keys = NULL;
			return -1;
		    }
		    *keys = tmp;
		}
		memcpy(*keys + (*num_keys)++,(PGPKEYID *) contents.data + j, 
		       sizeof(PGPKEYID));
	    }
	}

	rv = stringdb->seq(stringdb, &key, &contents, R_NEXT);
    }
    
    if(n) free(n);
    return 0;
}

struct search_info {
    int save_case;
    char *s;
};

static int keycompare(const void *info, const char *id)
{
    struct search_info *_info = (struct search_info *)info;
    char *s = strdup(id);
    char *u;
    int rv = 0;

    if(!s)
	return 0;
    
    if(!_info->save_case)
	for(u = s; *u; u++)
	    *u = toupper(*u);
    
    if(!*_info->s || strstr(s, _info->s))
	rv = 1;
    
    free(s);

    return rv;
}

/*
 * Find a string in the database.  We start by checking if we can
 * locate the exact string.  If not, we scan the database looking
 * for matches.
 * If save_case is TRUE we preserve case, if not, everything
 * is converted to UPPER.
 * Return an the number of KeyID's we found with a match (in num_keys),
 * and an array (at least) that big of the numbers.
 */

int
keys_from_string(char		*string,
		 int		save_case,
		 int		*num_keys,
		 PGPKEYID	**keys)
{
    DBT		key, contents;
    PGPKEYID	*keyp;
    int		ret_value;
    struct 	search_info si;

    char	*tmp;
    int		i;

    if ( string == NULL || num_keys == NULL || keys == 0) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    if ( ! dbase_open ) {
	pgplib_errno = PGPLIB_ERROR_DB_NOT_OPEN;
	return(-1);
    }

    *num_keys = 0;
    *keys = NULL;

    if(*string) {
	key.data = (void *)string;
	key.size = strlen(string) + 1;
	i = (stringdb->get)(stringdb, &key, &contents, 0);
	if ( i == -1 ) {
	    pgplib_errno = PGPLIB_ERROR_SYSERR;
	    return(-1);
	}
	if ( i == 0 ) {
	    /* Perfect match ! */
	    keyp = malloc(contents.size);
	    if ( keyp == NULL ) {
		pgplib_errno = PGPLIB_ERROR_SYSERR;
		return(-1);
	    }
	    memcpy(keyp, contents.data, contents.size);
	    *num_keys = contents.size/sizeof(PGPKEYID);
	    *keys = keyp;
	    return(0);
	}
    }

    si.save_case = save_case;
    if(!(si.s = strdup(string)))
    {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return -1;
    }

    if(!save_case)
    {
	for(tmp = si.s; *tmp; tmp++)
	    *tmp = toupper(*tmp);
    }

    ret_value = keys_from_function(keycompare, (void *) &si, num_keys, keys);
    
    free(si.s);
    return ret_value;
    
}

/*
 * Find the UIDs attached to this key.  Return of 1 means no UIDs
 * (or possibly unknown key ?), -1 is error, 0 is OK.
 * The array of pointers to UID's is NULL terminated.
 */

int
uids_from_keyid(PGPKEYID	key_id,
		PGPuserid_t	***uidpp)
{
    DBT		key, contents;
    int		offset;
    res_t	what;
    int		ent;
    int		used;
    PGPuserid_t	**arr, **tmp;
    /* scratch */
    int		i,j;

    if ( PGPKEYID_IsZero(key_id) || uidpp == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    if ( ! dbase_open ) {
	pgplib_errno = PGPLIB_ERROR_DB_NOT_OPEN;
	return(-1);
    }

    key.data = (void *)&key_id;
    key.size = sizeof(key_id);

    memset(&contents, 0 , sizeof(contents));

    i = (revstringdb->get)(revstringdb, &key, &contents, 0);
    if ( i == -1 )
	pgplib_errno = PGPLIB_ERROR_SYSERR;
    if ( i )
	return ( i ) ;

    /* Ensure our array is NULL terminated */
    if ( (arr = calloc(1, sizeof(PGPuserid_t *))) == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }

    ent = 1;
    offset = 0;
    do {
	tmp = realloc(arr, (ent+1) * sizeof(PGPuserid_t *));
	if ( tmp == NULL ) {
	    free(arr);
	    pgplib_errno = PGPLIB_ERROR_SYSERR;
	    return(-1);
	}
	arr = tmp;
	arr[ent] = NULL;
	i = get_keyentry_from_buf((u_char*)contents.data + offset,
				  contents.size - offset,
				  &what,
				  NULL, NULL, &arr[ent-1], NULL, NULL,
				  &used);
	switch ( i ) {
	case -1:
	    for(j=0; j<ent-2; j++)
		free(arr[j]);
	    free(arr);
	    pgplib_errno = PGPLIB_ERROR_SYSERR;
	    return(-1);
	case 1:
	    *uidpp = arr;
	    return(0);
	case 0:
	    /* the revstring database is suppoed to only hold UID's */
	    assert (what == UID);
	    offset += used;
	    ent += 1;
	    break;
	default:
	    assert(0);
	    /*NOTREACHED*/
	}
    } while (offset < contents.size);

    *uidpp = arr;
    return(0);
}

/*************************************************************
 * Now comes the higher level routines to access the databases.
 * They were moved here from keymgmnt.c after a suggestion
 * made by tzeruch@ceddec.com.
 *************************************************************
 */

/*
 * We find the key, and add the UserID, unless it is already present;
 * both the length and the characters themselves have to match (case
 * sensitive).
 * We add the UserID to the UserID database with the KeyID as
 * data.
 * We add the UserID to the reversUID database with the keyID as key
 * and UID as data.
 */

int
add_uid(PGPKEYID	key_id,
	PGPuserid_t	*user_id)
{
    u_char 	*rec;
    int		uid_len;
    res_t	what;
    int 	found;
    int 	sofar, buffer_len, used;
    int		found_pubkey;
    PGPuserid_t	*tmpuid;
    /* scratch */
    int		i;
    u_char	*p, *q;

    if ( user_id == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return -1;
    }

    i = get_pubkey_rec(key_id, &buffer_len, &rec);
    if ( i != 0 ) {
	return(i);
    }

    /* We parse the buffer, skipping things we don't care about,
     * looking for a matching UserID.
     * if we don't find a match, we add this at the end.
     */
    found = 0;
    found_pubkey = 0;
    sofar = 0;
    while ((sofar < buffer_len) && (found == 0))
    {
	i = get_keyentry_from_buf(rec + sofar, buffer_len - sofar, &what,
				  NULL, NULL, &tmpuid, NULL, NULL, &used);

	if (i == 1 || i == -1)
	    return -1;
	sofar += used;

	if (what == UID)
	{
	    if(!strcmp(user_id->name, tmpuid->name))
		found = 1;
	    free_uid(tmpuid);
	    free(tmpuid);
	}
    } /* While */

    if ( found == 0 ) {

	/* If we get here, we have a new UserID.  Let's add it. */
	i  = uid2buf(user_id, &p);
	if ( i  != 0 )
	    return(-1);
	uid_len = packet_length(p);
	if ( uid_len == -1 ) {
	    free(p);
	    free(rec);
	    return(-1);
	}
	q = realloc(rec, buffer_len + uid_len);
	if ( q == NULL ) {
	    free(p);
	    free(rec);
	    return(-1);
	}
	rec = q;
	memcpy(rec + buffer_len, p, uid_len);
	free(p);
	i = store_pubkey_rec(key_id, REPLACE, rec,
			     buffer_len + uid_len);
	if ( i != 0 ) {
	    return(-1);
	}
    }
    free(rec);

    /* Then, store the string in the string_db with the KeyID as data.
     * this ensures fast lookup.  We try even if FOUND==TRUE, since we
     * might update the database.
     */
    i = store_uid(user_id, key_id);
    if ( i == 1 && found != 1)
	return(i);
    return(0);
}

/* Add a trust packet to a public key.
 * The type of the trust packet depends on the
 * uid parameter: uid == NULL implies that we have
 * a key trust packet, while uid != NULL means
 * that we are talking about the trust in a 
 * specific key-user id binding.
 */

int
add_trust(PGPKEYID	key_id,
	  PGPuserid_t	*uid,
	  PGPtrust_t	*trust)
{
    u_char *buf;    
    u_char *p;
    u_char *tbuf;
    int p_len;
    int buffer_len;
    int i;
    int sofar;
    res_t what;
    int used;
    PGPuserid_t *tmpuid;
    int found;

    if( PGPKEYID_IsZero(key_id) || trust == 0 )
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return -1;
    }

 retry:
    
    i = get_pubkey_rec(key_id, &buffer_len, &buf);
    if ( i != 0 ) {
	return(i);
    }

    sofar = 0;

    if(uid) {
	found = 0;
	while(sofar < buffer_len && !found)
	{
	    i = get_keyentry_from_buf(buf + sofar, buffer_len - sofar, &what,
				      NULL, NULL, &tmpuid, NULL, NULL, &used);
	    
	    if(i == -1)
		return -1;
	    else if(i)
		break;

	    sofar += used;

	    if(what == UID)
	    {
		if(!strcmp(tmpuid->name, uid->name))
		    found = 1;
		free_uid(tmpuid);
	    } 
	}

	if(!found) { 		 /* add this user-ID */
	    i = add_uid(key_id, uid);
	    free(buf);
	    if(i) return i;
	    goto retry;
	}

    } else {
	sofar = packet_length(buf);
    }
    
    if(sofar < buffer_len)
    {
	if(is_ctb(buf[sofar]) && is_ctb_type(buf[sofar], CTB_KEYCTRL_TYPE))
	{
	    p_len = packet_length(buf + sofar);
	    memmove(buf + sofar, buf + sofar + p_len,
		    buffer_len - p_len - sofar);
	    buffer_len -= p_len;
	}
    }

    trust2buf(trust, &tbuf);
    
    if((p = realloc(buf, buffer_len + (p_len = packet_length(tbuf)))) == NULL)
    {
	int rn = errno;
	free(buf);
	free(tbuf);
	errno = rn;
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return -1;
    }

    buf = p;

    memmove(buf + sofar + p_len, buf + sofar, buffer_len - sofar);
    memcpy (buf + sofar, tbuf, p_len);

    i = store_pubkey_rec(key_id, REPLACE, buf, buffer_len+p_len);
    free(buf);
    free(tbuf);

    return i;
}


/*
 * Adding a signature to a key, verifying that it isn't already
 * there.
 * If we added, return 0.  No adding returns 1, error gives -1.
 */

int
add_sig(PGPKEYID	key_id,
	PGPuserid_t 	*uid,
	PGPsig_t 	*sig)
{

    u_char 	*buf;
    int 	found;
    int		buffer_len, sofar, used;
    res_t	what;
    u_char	*sig_buf;
    int		sig_len;
    PGPsig_t	*tmpsig;
    PGPuserid_t *tmpuid;

    /* scratch */
    u_char	*p;
    int 	i, j;

    if(!sig) 
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return -1;
    }

    if((sig->classification >= K0_SIGNATURE_BYTE 
	&& sig->classification <= K3_SIGNATURE_BYTE)
       || sig->classification == KR_SIGNATURE_BYTE)
    {
	if(!uid)
	{
	    pgplib_errno = PGPLIB_ERROR_EINVAL;
	    return -1;
	}
    } 
    else 
	uid = NULL;

 retry:
    
    i = get_pubkey_rec(key_id, &buffer_len, &buf);
    if ( i != 0 ) {
	return(i);
    }

    sofar = 0;

    if(uid) {
	for(found = 0; sofar < buffer_len && !found; ) {
	    i = get_keyentry_from_buf(buf + sofar, buffer_len - sofar, 
				      &what, NULL, NULL, &tmpuid, 
				      NULL, NULL, &used);
	    
	    sofar += used;
	    if(i == -1)
		return -1;
	    else if(i)
		break;

	    
	    if(what == UID) {
		if(!strcmp(uid->name, tmpuid->name))
		    found = 1;
		free_uid(tmpuid);
	    }
	}

	if(!found) { 		 /* add this user-ID */
	    i = add_uid(key_id, uid);
	    free(buf);
	    if(i) return i;
	    goto retry;
	}
    }

    found = 0;

    while ( sofar < buffer_len && found == 0 ) {
	i = get_keyentry_from_buf( buf + sofar, buffer_len - sofar, &what, 
				   &tmpsig, NULL, NULL, NULL, NULL, &used);
	if ( i == 1 || i == -1 )
	    return(-1);

	if (what == UID)
	    break;
	else
	    sofar += used;
	
	if (what != SIG)
	    continue;

	/* We must verify that the signature at hand is different from
	 * the one already on the key.  We first check if it is made
	 * in the same date, then if it is made with the same key
	 * (long comparison is faster than quad).  If both matches,
	 * we dig out the actual signatures and compare them.
	 */
	
	/* assert(tmpsig != NULL);*/
	
	if (tmpsig->timestamp == sig->timestamp 
	    && PGPKEYID_EQUAL(tmpsig->key_id, sig->key_id)
	    && tmpsig->classification == sig->classification)
	{
	    j = BN_cmp(tmpsig->I, sig->I);
	    if ( j == 0 ) {
		/* A copy */
		found = 1;
	    }
	    
	}
	free_signature(tmpsig);
    }
    
    if ( found == 1 ) {
	/* It was already there */
	free(buf);
	return(1);
    }

    /* This signature wasn't on the key.  Let us add it. */
    i = signature2buf(sig, &sig_buf);
    if ( i != 0 ) {
	free(buf);
	return(-1);
    }

    if((sig_len = packet_length(sig_buf)) == -1) {
	free(buf);
	return(-1);
    }

    if((p = realloc(buf, buffer_len + sig_len)) == NULL)
    {
	int rn = errno;
	if(buf) free(buf);
	errno = rn;
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    buf = p;

    memmove(buf + sofar + sig_len, buf + sofar, buffer_len - sofar);
    memcpy(buf + sofar, sig_buf, sig_len);
    
    free(sig_buf);
    j = store_pubkey_rec(key_id, REPLACE, buf, buffer_len+sig_len);
    free(buf);
    return j;
}



/*
 * In order to fetch just a public key, we fetch everything
 * stored on this keyID,  Then copy out the key proper and
 * free'ing the rest.
 * Return -1 on error, 1 on no-such-key and 0 on OK.
 */

int
get_only_pubkey(PGPKEYID 	key_id,
		PGPpubkey_t 	**key)
{
    keyent_t	**recp;
    PGPpubkey_t	*tmpkey;
    int		index;
    /* scratch */
    int 	i;

    if ( key == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    i = get_keycontents(key_id, &recp);
    if ( i == -1 || i == 1 )
	return(i);

    tmpkey = calloc(1, sizeof(PGPpubkey_t));
    if ( tmpkey == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    for ( index = 0; recp[index] != NULL; index++) {
	switch (recp[index]->what) {
	case TRUSTBYTE:
	case SIG:
	case UID:
	    continue;
	case PUBKEY:
	    i = copy_pubkey(recp[index]->u.key, tmpkey);
	    if ( i == -1 ) {
		free_keyrecords(recp);
		return(-1);
	    }
	    continue;
	default:
	    assert(0);
	    /*NOTREACHED*/
	}
    }
    free_keyrecords(recp);
    *key = tmpkey;

    return(0);
}


/*
 * Get a public key, and parse out the different packages that
 * we find there.  Return a pointer to an array of pointers to
 * structures.  Everything is malloc'ed and must be freed by the
 * caller; call free_keyentry (see below).
 * A return of 1 means no such key, -1 error, 0=OK.
 */

int
keybuf_to_records(u_char *keybuf, int buf_len, keyent_t ***keyent)
{
    int		used;
    keyent_t	**arr;
    int		ent;
    int		offset;
    res_t	what;
    PGPsig_t	*sig;
    PGPpubkey_t	*key;
    PGPuserid_t	*uid;
    PGPtrust_t	*trust;
    int		ret;
    char	*p;


    if(!keybuf || !buf_len || !keyent)
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return -1;
    }

    
    /* Ensure our array is NULL terminated */
    arr = calloc(1, sizeof(keyent_t **));
    if ( arr == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }

    ent = 0;
    offset = 0;
    used = 0;

    while ( offset < buf_len ) {
	ret = get_keyentry_from_buf(keybuf+offset,
				    buf_len-offset,
				    &what, &sig, &key,
				    &uid, NULL, &trust, &used);
	if ( ret != 0 ) {
	    return(-1);
	}

	offset += used;

	p = realloc(arr, (ent+2) * sizeof(keyent_t *));
	if ( p == NULL ) {
	    pgplib_errno = PGPLIB_ERROR_SYSERR;
	    free(arr);
	    return(-1);
	}
	arr = (keyent_t **)p;
	arr[ent+1] = NULL;

	arr[ent] = malloc(sizeof(keyent_t));
	if ( arr[ent] == NULL ) {
	    pgplib_errno = PGPLIB_ERROR_SYSERR;
	    return(-1);
	}
	arr[ent]->what = what;

	switch (what) {
	default:
	case NONE:
	    assert(0);
	case PUBKEY:
	    arr[ent]->u.key = key;
	    break;
	case SIG:
	    arr[ent]->u.sig = sig;
	    break;
	case UID:
	    arr[ent]->u.uid = uid;
	    break;
	case TRUSTBYTE:
	    arr[ent]->u.trust = trust;
	    break;
	}
	ent += 1;
    }

    *keyent = arr;
    return(0);
}

int
get_keycontents_seq(keyent_t ***keyent, int where)
{
    DBT key, data;
    int rv;

    if((rv = keydb->seq(keydb, &key, &data, where)) != 0)
	return rv;

    return keybuf_to_records(data.data, data.size, keyent);
}
    

int
get_keycontents(PGPKEYID	key_id,
		keyent_t	***keyent)
{
    int		buf_len;
    u_char	*keybuf;
    int		i;

    if ( keyent == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    i = get_pubkey_rec(key_id, &buf_len, &keybuf);
    if ( i != 0 )
	return(i);

    i = keybuf_to_records(keybuf, buf_len, keyent);
    free(keybuf);
    return i;
}


/* we expect a public key record. */

int
add_pubkey_rec(u_char 	*buf, 
	       int 	size)
{
    u_char digest[16];
    PGPpubkey_t pk;
    int new_key;
    DBT key, data;
    PGPKEYID key_id;
    int i;
    int used;
    PGPsig_t *sig, *tmpsig = NULL;
    PGPpubkey_t *pubkey, *tmppubkey = NULL;
    PGPuserid_t *uid, *tmpuid = NULL;
    PGPtrust_t *trust;
    res_t what;


    if((i = buf2pubkey(buf, size, &pk)))
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return 1;
    }

    MD5(buf, size, digest);

    if ( PGPKEYID_FromBN(&key_id, pk.N)) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
       	return 1;
    }

    free_pubkey(&pk);

    key.data = &key_id;
    key.size = sizeof(key_id);

    /* anything to do? */

    if((new_key = keyhashdb->get(keyhashdb, &key, &data, 0)) == 0)
    {
	if(data.size == sizeof(digest) && 
	   !memcmp(digest, data.data, data.size))
	    return 0;
    }

    if(new_key)
    {
	if((i = store_pubkey_rec(key_id, REPLACE, buf, size)))
	    return i;
    }

    used = 0;
    i = 0;
    while(used <  size &&
	  get_keyentry_from_buf(buf + i, size - i,
				&what, &sig, &pubkey, &uid, NULL,
				&trust, &used) == 0)
    {
	i += used;
	switch(what)
	{
	case PUBKEY:
	    if(tmpsig) 
	    {
		free_signature(tmpsig);
		free(tmpsig);
		tmpsig = NULL;
	    }
	    
	    if(tmpuid)
	    {
		free_uid(tmpuid);
		free(tmpuid);
		tmpuid = NULL;
	    }
      
	    if(tmppubkey)
	    {
		free_pubkey(tmppubkey);
		free(tmppubkey);
	    }
	    
	    tmppubkey = pubkey;

	    break;

	case UID:
	    if(tmpsig)
	    {
		free_signature(tmpsig);
		free(tmpsig);
		tmpsig = NULL;
	    }
      
	    if(tmpuid)
	    {
		free_uid(tmpuid);
		free(tmpuid);
	    }
	    tmpuid = uid;
	    
	    if(tmppubkey && !new_key) 
		add_uid(key_id, tmpuid);
	    else if(tmppubkey)
		store_uid(tmpuid, key_id);

	    break;

	case SIG:
	    if(tmpsig)
	    {
		free_signature(tmpsig);
		free(tmpsig);
		tmpsig = NULL;
	    }
	    tmpsig = sig;
	    if(tmppubkey && !new_key)
		add_sig(key_id, tmpuid, tmpsig);
	    
	    break;

	case TRUSTBYTE:
	    if(!tmpsig && tmppubkey && tmpuid && !new_key)
		add_trust(key_id, tmpuid, trust);

	    break;
	    
	default:
	    /* shut up the compiler */
	    break;
	    
	}
    }

    if(tmpuid) {
	free_uid(tmpuid);
	free(tmpuid);
    }

    if(tmppubkey) {
	free_pubkey(tmppubkey);
	free(tmppubkey);
    }

    if(tmpsig) {
	free_signature(tmpsig);
	free(tmpsig);
    }

    return 0;
}

static u_char *enlarge(u_char *buff, int needed, int *now)
{
    u_char *scratch;

    if(needed <= *now)
	return buff;

    if((scratch = realloc(buff, needed)) == NULL)
    {
	free(buff);
	return NULL;
    }

    *now = needed;

    return scratch;
}


/* scan the public key ring, avoiding mallocs and reallocs wherever
 * possible. */

int 
scan_pubring(FILE *fp)
{
    u_char *packet = NULL;
    u_char *pkr = NULL;
    u_char last_ctb = 0;
    int p_len;
    int i;
    int pl = 0;
    int kl = 0;
    int ll;

    int pu = 0;
    int ku = 0;

    packet = malloc(1);
    pl = 1;

    while(1)
    {
	if((pu = fread(packet, 1, 1, fp)) < 1 || !is_ctb(packet[0]))
	    break;

	if((ll = ctb_llength(packet[0])) < 0)
	    break;

	if(!(packet = enlarge(packet, pu + ll, &pl)))
	    break;

	if((i = fread(packet + pu, 1, ll, fp)) < ll || 
	    (p_len = packet_length(packet)) < 0)
	    break;
	
	pu += i;

	if(!(packet = enlarge(packet, p_len, &pl)))
	    break;

	i = p_len - pu;

	if(fread(packet+pu, 1, i, fp) < i)
	    break;

	pu += i;

	if(is_ctb_type(packet[0], CTB_KEYCTRL_TYPE))
	{
	    if(last_ctb && is_ctb_type(last_ctb, CTB_USERID_TYPE))
	    {
		if((pkr = enlarge(pkr, ku + p_len, &kl)) == NULL)
		    break;
	
		memcpy(pkr + ku, packet, p_len);
		ku += p_len;
	    }
	}
	else 
	{
	    if(pkr && ku && is_ctb_type(packet[0], CTB_CERT_PUBKEY_TYPE))
	    {
		add_pubkey_rec(pkr, ku);
		ku = 0;
	    } 
	    
	    if((pkr = enlarge(pkr, ku + p_len, &kl)) == NULL)
		break;
	    
	    memcpy(pkr + ku, packet, p_len);
	    ku += p_len;
	}
	last_ctb = packet[0];
    }

    if(pkr && ku)
	add_pubkey_rec(pkr, ku);

    if(pkr) free(pkr);
    if(packet) free(packet);

    if(!feof(fp))
	return -1;
    else
	return 0;
}
